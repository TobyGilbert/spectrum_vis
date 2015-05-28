#include "base.h"
#include <string>
#include <vector>

#define gldbg assert(glGetError() == GL_NO_ERROR);
float t = 0;

// number of freq spectrum bands rendered
#define N (1024)

GLuint GPU_vertex_shader, GPU_fragment_shader, GPU_render_program;

GLuint spectrum_vao, spectrum_vbo;

// vertex shader
std::string vs_src(
	"#version 330											\n"
	"														\n"
	"uniform float time;									\n"
	"layout (location = 0) in vec2 a_pos;					\n"
	"out vec2 pos;											\n"
	"														\n"
	"void main(void)										\n"
	"{														\n"
	"	pos = a_pos;pos.y -= 0.98;							\n"
	"	gl_Position.xy = pos;"
	"}														\n"
);

// fragment shader
std::string fs_src(""
"#version 330											\n"
"														\n"
"uniform vec2 resolution;								\n"
"uniform float time;									\n"
"														\n"
"in vec2 pos;											\n"
"out vec4 frag_col;										\n"
"														\n"
"void main(void)										\n"
"{														\n"
"	frag_col = vec4(abs(pos.y), abs(sin(time)), abs(pos.x), 1.0f);	\n"
"}														\n"
);

HSTREAM streamHandle; // Handle for open stream
DWORD chan;
float stream_fft_output_buf[N];

const char* file = rdir_ "aud.mp3";

struct float2
{
	float x, y;
};

bool fspec_init(void) {

  if (!(chan = BASS_StreamCreateFile(FALSE, file, 0, 0, BASS_SAMPLE_LOOP)) &&!(
          chan = BASS_MusicLoad(FALSE, file, 0, 0,
		  BASS_MUSIC_RAMPS | BASS_SAMPLE_LOOP | BASS_SAMPLE_MONO, 1))) {
    fprintf(stderr, "abort: can't play file");
    return FALSE; // Can't load the file
  }

  BASS_ChannelPlay(chan, FALSE);

  BASS_CHANNELINFO stream_channel_info;
  BASS_ChannelGetInfo(chan, &stream_channel_info); // get number of channels

  struct {
    void operator()(GLint shader) {
      GLint isCompiled = 0;
      glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
      if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::string infoLog;
        infoLog.resize(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

        // We don't need the shader anymore.
        glDeleteShader(shader);

        // Use the infoLog as you see fit.
        fprintf(stderr, "%s", infoLog.data());

        // In this simple program, we'll just leave
        assert(false && "shader compilation failure");
      }
    }
  } check_is_compiled;

  GPU_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  const char *vs_str = vs_src.c_str();
  glShaderSource(GPU_vertex_shader, 1, &vs_str, NULL);
  gldbg;
  glCompileShader(GPU_vertex_shader);
  gldbg;
  check_is_compiled(GPU_vertex_shader);

  GPU_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  const char *fs_str = fs_src.c_str();
  glShaderSource(GPU_fragment_shader, 1, &fs_str, NULL);
  gldbg;
  glCompileShader(GPU_fragment_shader);
  gldbg;
  check_is_compiled(GPU_fragment_shader);

  GPU_render_program = glCreateProgram();
  glAttachShader(GPU_render_program, GPU_vertex_shader);
  gldbg;
  glAttachShader(GPU_render_program, GPU_fragment_shader);
  gldbg;
  // glValidateProgram(GPU_render_program); gldbg;
  glLinkProgram(GPU_render_program);
  gldbg;

  GLint isLinked = 0;
  glGetProgramiv(GPU_render_program, GL_LINK_STATUS, (int *)&isLinked);
  if (isLinked == GL_FALSE) {
    GLint maxLength = 0;
    glGetProgramiv(GPU_render_program, GL_INFO_LOG_LENGTH, &maxLength);

    // The maxLength includes the NULL character
    std::string infoLog;
    infoLog.resize(maxLength);
    glGetProgramInfoLog(GPU_render_program, maxLength, &maxLength, &infoLog[0]);

    // We don't need the program anymore.
    glDeleteProgram(GPU_render_program);
    // Don't leak shaders either.
    glDeleteShader(GPU_vertex_shader);
    glDeleteShader(GPU_fragment_shader);

    // Use the infoLog as you see fit.

    assert(false && "render program compilation failure");
  }

  glDetachShader(GPU_render_program, GPU_vertex_shader);
  glDetachShader(GPU_render_program, GPU_fragment_shader);

  // simply bind once as it is the only GPU program used by application
  glUseProgram(GPU_render_program);

  GLint l = glGetUniformLocation(GPU_render_program, "resolution");
  glProgramUniform2f(GPU_render_program, l, (GLfloat)g_window_width,
                     (GLfloat)g_window_height);
  gldbg;

  glGenVertexArrays(1, &spectrum_vao);
  glBindVertexArray(spectrum_vao);

  glGenBuffers(1, &spectrum_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, spectrum_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float2)* N, NULL, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glBindBuffer(GL_ARRAY_BUFFER, spectrum_vbo);
  float2* gpu_mem_ptr = (float2*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
  assert(gpu_mem_ptr != NULL && "mapped pointer is NULL");

  float step = 1.0f / (N/2);
  for (int i = 0; i < N - 1; ++i)
  {
	  gpu_mem_ptr[i].x = (step * ((N / 2) - i));
	  gpu_mem_ptr[i].y = 0;
  }

  GLboolean b = glUnmapBuffer(GL_ARRAY_BUFFER);
  assert(b == GL_TRUE && "failed to unmapped buffer pointer");
  gpu_mem_ptr = NULL;


  /*glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);*/

  glPointSize(2.0);

  return true;
}

void fspec_teardown(void)
{	
	glDeleteBuffers(1, &spectrum_vbo);
	glDeleteVertexArrays(1, &spectrum_vao);
}

void fspec_update(float dt)
{
	t += dt * 15;
	GLint l = glGetUniformLocation(GPU_render_program, "time");
	glProgramUniform1f(GPU_render_program, l, t);

	// get the sample data (floating-point to avoid 8 & 16 bit processing)
	BASS_ChannelGetData(chan, stream_fft_output_buf, BASS_DATA_FFT16384 | BASS_DATA_FLOAT);

	glBindBuffer(GL_ARRAY_BUFFER, spectrum_vbo);
	float2* gpu_mem_ptr = (float2*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	assert(gpu_mem_ptr != NULL && "mapped pointer is NULL");
	
	for (int i = 0; i < N/2; ++i)
	{
		float s = (float)i / (N / 2);
		// magnitude
		float mag = stream_fft_output_buf[i];
		if (mag > 0.0002)
			mag *= 109067.1f * (1+(s*5.0f));

		gpu_mem_ptr[(N/2) - i ].y = mag * dt;

		gpu_mem_ptr[(N/2) + i].y = mag  * dt;
	}

	GLboolean b = glUnmapBuffer(GL_ARRAY_BUFFER);
	assert(b == GL_TRUE && "failed to unmapped buffer pointer");
	gpu_mem_ptr = NULL;
}

void fspec_render(void)
{	
	glBindVertexArray(spectrum_vao);
	glEnableVertexArrayAttrib(spectrum_vao, 0);

	glDrawArrays(GL_POINTS, 0, N);

	glDisableVertexArrayAttrib(spectrum_vao, 0);
	glBindVertexArray(0);
}
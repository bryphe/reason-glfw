external print_hello: unit => unit = "caml_print_hello";

type window;

/* GLFW */
external glfwInit: unit => bool = "caml_glfwInit";
external glfwCreateWindow: (int, int, string) => window =
  "caml_glfwCreateWindow";
external glfwMakeContextCurrent: window => unit =
  "caml_glfwMakeContextCurrent";
external glfwWindowShouldClose: window => bool = "caml_glfwWindowShouldClose";
external glfwPollEvents: unit => unit = "caml_glfwPollEvents";
external glfwTerminate: unit => unit = "caml_glfwTerminate";
external glfwSwapBuffers: window => unit = "caml_glfwSwapBuffers";

/* GL */
type shader;
type shaderType =
  | GL_VERTEX_SHADER
  | GL_FRAGMENT_SHADER;

external glClearColor: (float, float, float, float) => unit =
  "caml_glClearColor";
external glClearDepth: float => unit = "caml_glClearDepth";
external glCreateShader: shaderType => shader = "caml_glCreateShader";
external glShaderSource: (shader, string) => unit = "caml_glShaderSource";

type enableOptions =
  | GL_DEPTH_TEST;

external glEnable: enableOptions => unit = "caml_glEnable";

type depthFunctions =
  | GL_LEQUAL;

external glDepthFunc: depthFunctions => unit = "caml_glDepthFunc";

/* TODO: Add compile result return */
external glCompileShader: shader => unit = "caml_glCompileShader";
external glDeleteShader: shader => unit = "caml_glDeleteShader";
/* TODO: Remove */
external glGetShaderIsCompiled: shader => bool = "caml_glGetShaderIsCompiled";

type program;

external glCreateProgram: unit => program = "caml_glCreateProgram";
external glAttachShader: (program, shader) => unit = "caml_glAttachShader";
external glUseProgram: program => unit = "caml_glUseProgram";

/* TODO: Add link result return */
external glLinkProgram: program => unit = "caml_glLinkProgram";

type attribLocation;
external glGetAttribLocation: (program, string) => attribLocation =
  "caml_glGetAttribLocation";

type uniformLocation;
external glGetUniformLocation: (program, string) => uniformLocation =
  "caml_glGetUniformLocation";
external glUniformMatrix4fv: uniformLocation => /* todo */ unit =
  "caml_glUniformMatrix4fv";

type bufferType =
  | GL_ARRAY_BUFFER;

type buffer;
external glCreateBuffer: unit => buffer = "caml_glCreateBuffer";
external glBindBuffer: (bufferType, buffer) => unit = "caml_glBindBuffer";
external glUnbindBuffer: bufferType => unit = "caml_glUnbindBuffer";

type drawType =
  | GL_STATIC_DRAW;

external glBufferData:
  (
    bufferType,
    Bigarray.Array1.t(float, Bigarray.float32_elt, Bigarray.c_layout),
    drawType
  ) =>
  unit =
  "caml_glBufferData";

type glType =
  | GL_FLOAT;

external glVertexAttribPointer: (attribLocation, int, glType, bool) => unit =
  "caml_glVertexAttribPointer";
external glEnableVertexAttribArray: attribLocation => unit =
  "caml_glEnableVertexAttribArray";

type drawMode =
  | GL_TRIANGLES;

external glDrawArrays: (drawMode, int, int) => unit = "caml_glDrawArrays";

external printFrameBufferSize: window => unit = "caml_printFrameBufferSize";
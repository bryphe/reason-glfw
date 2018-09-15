open Glfw

let loadShader shaderType source =
    let shader = glCreateShader shaderType in
    let () = glShaderSource shader source in
    let () = glCompileShader shader in
    let _ = glGetShaderIsCompiled shader in
    shader
;;

let initShaderProgram vsSource fsSource =
    let vsShader = loadShader GL_VERTEX_SHADER vsSource in
    let fsShader = loadShader GL_FRAGMENT_SHADER fsSource in
    let shaderProgram = glCreateProgram () in
    let () = glAttachShader shaderProgram vsShader in
    let _ = glAttachShader shaderProgram fsShader in
    let _ = glLinkProgram shaderProgram in
    shaderProgram
;;

let () =
    print_hello ();
    let _ = glfwInit () in
    let w = glfwCreateWindow 800 600 "test" in
    glfwMakeContextCurrent(w);

    let vsSource = {|
        #ifndef GL_ES
        #define lowp
        #endif

        attribute vec3 aVertexPosition;
        attribute vec4 aVertexColor;

        uniform mat4 transform;

        varying lowp vec4 vColor;

        void main() {
            gl_Position = transform * vec4(aVertexPosition, 1.0);
            vColor = aVertexColor;
        }
    |} in
    print_endline(vsSource);
    let fsSource = {|
        #ifndef GL_ES
        #define lowp
        #endif

        varying lowp vec4 vColor;

        void main() {
            gl_FragColor = vColor;
        }
    |} in
    print_endline(fsSource);

    let positions = [|-0.5; -0.5; 0.0; 0.5; -0.5; 0.0; 0.0; 0.5; 0.0|] in
    let colors = [|1.0; 0.0; 0.0; 1.0; 0.0; 1.0; 0.0; 1.0; 0.0; 0.0; 1.0; 1.0|] in
    let vArray = Bigarray.Array1.of_array Bigarray.Float32 Bigarray.C_layout positions in
    let cArray = Bigarray.Array1.of_array Bigarray.Float32 Bigarray.C_layout colors in
    let shaderProgram = initShaderProgram vsSource fsSource in
    let vb = glCreateBuffer () in
    let cb = glCreateBuffer () in
    glBindBuffer GL_ARRAY_BUFFER vb;
    glBufferData GL_ARRAY_BUFFER vArray GL_STATIC_DRAW;

    glBindBuffer GL_ARRAY_BUFFER cb;
    glBufferData GL_ARRAY_BUFFER cArray GL_STATIC_DRAW;
    let posAttribute = glGetAttribLocation shaderProgram "aVertexPosition" in
    let colorAttribute = glGetAttribLocation shaderProgram "aVertexColor" in
    let worldUniform = glGetUniformLocation shaderProgram "transform" in
    while not (glfwWindowShouldClose w) do
        glClearColor 0.0 0. 0. 1.;
        glClearDepth 1.0;
        glEnable GL_DEPTH_TEST;
        glDepthFunc GL_LEQUAL;

        glUseProgram shaderProgram;
        glUniformMatrix4fv worldUniform;
        glBindBuffer GL_ARRAY_BUFFER vb;
        glVertexAttribPointer posAttribute 3 GL_FLOAT false;
        glEnableVertexAttribArray posAttribute;

        glBindBuffer GL_ARRAY_BUFFER cb;
        glVertexAttribPointer colorAttribute 4 GL_FLOAT false;
        glEnableVertexAttribArray colorAttribute;
        glDrawArrays GL_TRIANGLES 0 3;

        glfwSwapBuffers(w);
        glfwPollEvents();
    done;
    print_endline "Done!";
    glfwTerminate ();
;;
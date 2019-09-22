//open Reglfw;
open Reglfw.Glfw;

let isNative =
  switch (Sys.backend_type) {
  | Native => true
  | Bytecode => true
  | _ => false
  };

let getExecutingDirectory = () =>
  isNative ? Filename.dirname(Sys.argv[0]) ++ Filename.dir_sep : "";

let width = ref(0);
let height = ref(0);

let run = () => {
  let _ = glfwInit();
  let primaryWindow = glfwCreateWindow(100, 50, "test");
  glfwMakeContextCurrent(primaryWindow);

  let monitor = glfwGetPrimaryMonitor();
  let vidMode = glfwGetVideoMode(monitor);

  glfwSetWindowPos(
    primaryWindow,
    (vidMode.width - 800) / 2,
    (vidMode.height - 600) / 2,
  );
  glfwSetWindowSize(primaryWindow, 800, 600);
  glfwSetWindowTitle(primaryWindow, "reason-glfw example");

/*   let secondaryWindow = */
/*     glfwCreateWindow(100, 50, ~sharedContext=primaryWindow, "secondWindow"); */
/*   glfwSetWindowPos(secondaryWindow, vidMode.width / 2, vidMode.height / 2); */
/*   glfwSetWindowSize(secondaryWindow, 800, 600); */
/*   glfwSetWindowTitle(secondaryWindow, "second window"); */

  let cursors = [|
    glfwCreateStandardCursor(GLFW_ARROW_CURSOR),
    glfwCreateStandardCursor(GLFW_IBEAM_CURSOR),
    glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR),
    glfwCreateStandardCursor(GLFW_HAND_CURSOR),
    glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR),
    glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR),
  |];
  Random.self_init();
  let cursor = Random.int(Array.length(cursors));
  glfwSetCursor(primaryWindow, cursors[cursor]);

  glfwSetWindowIcon(primaryWindow, getExecutingDirectory() ++ "favicon01.png");

  //glViewport(0, 0, 800, 600);

  /* glfwSwapInterval sets the 'swap interval' - also known as vsync / vertical synchronization */
  /* A setting of 0 disables vsync, unless it is overridden by a driver or a user setting */
  glfwSwapInterval(0);

  let render = window => {
    open Skia;
    glfwMakeContextCurrent(window);
    print_endline("Here");
    let width = 640;
    let height = 480;
    let glContext = Skia.Gr.Context.makeGl(None);
    switch (glContext) {
    | None => prerr_endline ("No context");
    | Some(glContext) => 
      let framebufferInfo = Gr.Gl.FramebufferInfo.make(Unsigned.UInt.of_int(0), Unsigned.UInt.of_int(0x8058));
      let backendRenderTarget  = Gr.BackendRenderTarget.makeGl(width, height, 0, 8, framebufferInfo);
      let surfaceProps = SurfaceProps.make(Unsigned.UInt32.of_int(0), Unknown);
      
      switch(Surface.makeFromBackendRenderTarget(glContext, backendRenderTarget, TopLeft, Rgba8888, None, Some(surfaceProps))) {
      | None => prerr_endline("NO surface");
      | Some(surface) =>
        print_endline ("Trying to draw....");
        let canvas = Surface.getCanvas(surface);
          let fill = Paint.make();
          Paint.setColor(fill, Color.makeArgb(0xFF, 0x00, 0x00, 0xFF));
          Canvas.drawPaint(canvas, fill);

          Paint.setColor(fill, Color.makeArgb(0xFF, 0x00, 0xFF, 0xFF));
          let rect = Rect.makeLtrb(100., 100., 540., 380.);
          Canvas.drawRect(canvas, rect, fill);

          Canvas.flush(canvas);
        }
    }
    /*switch (ctx) {
    | None => prerr_endline ("No context");
    | Some(_) => prerr_endline ("Got context");
    }*/
    glfwSwapBuffers(window);
  };


  /* glfwMaximizeWindow(w); */

  let nativeWindow = glfwGetNativeWindow(primaryWindow);
  print_endline ("Native window handle/pointer: " ++ string_of_int(Obj.magic(nativeWindow)));

  let frame = ref(0);
  glfwRenderLoop(_t => {
    if (frame^ == 0) {
      render(primaryWindow);
    }
    Gc.full_major();
    incr(frame);

    glfwPollEvents();
    glfwWindowShouldClose(primaryWindow);
  });

  print_endline("Done!");
  glfwTerminate();
};

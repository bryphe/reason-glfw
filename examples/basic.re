open Reglfw;
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
    glfwMakeContextCurrent(window);
    glfwSwapBuffers(window);
  };

  glfwSetScrollCallback(primaryWindow, (_w, deltaX, deltaY) =>
    print_endline(
      "SCROLL: "
      ++ string_of_float(deltaX)
      ++ ", "
      ++ string_of_float(deltaY),
    )
  );

  glfwSetCursorPosCallback(primaryWindow, (_w, x, y) =>
    print_endline(
      "CURSOR: " ++ string_of_float(x) ++ ", " ++ string_of_float(y),
    )
  );

  glfwSetKeyCallback(
    primaryWindow,
    (_w, _key, _scancode, buttonState, m) => {
      print_endline(
        "KEY: "
        ++ string_of_int(Obj.magic(_key))
        ++ "| " ++ Modifier.show(m)
        ++ "| state: "
        ++ ButtonState.show(buttonState),
      );

      if (_key == GLFW_KEY_X) {
        glfwDestroyWindow(_w);
      };
    },
  );

  glfwSetCharCallback(primaryWindow, (_w, codepoint) =>
    print_endline(
      "CHAR: "
      ++ string_of_int(codepoint)
      ++ " | "
      ++ String.make(1, Uchar.to_char(Uchar.of_int(codepoint))),
    )
  );

  glfwSetCharModsCallback(
    primaryWindow,
    (_w, codepoint, m) => {
      print_endline(
      "CHAR MODS: "
      ++ string_of_int(codepoint)
      ++ " | "
      ++ String.make(1, Uchar.to_char(Uchar.of_int(codepoint)))
      ++ " | " 
      ++ Modifier.show(m)
      );
    }
  );

  glfwSetCharCallback(primaryWindow, (_w, codepoint) =>
    print_endline(
      "CHAR: "
      ++ string_of_int(codepoint)
      ++ " | "
      ++ String.make(1, Uchar.to_char(Uchar.of_int(codepoint))),
    )
  );

  glfwSetMouseButtonCallback(
    primaryWindow,
    (_w, button, buttonState, m) => {
      let controlPressed = string_of_bool(Modifier.isControlPressed(m));
      let shiftPressed = string_of_bool(Modifier.isShiftPressed(m));
      print_endline(
        "Mouse button: "
        ++ MouseButton.show(button)
        ++ "| ctrl: "
        ++ controlPressed
        ++ " | shift: "
        ++ shiftPressed
        ++ "| state: "
        ++ ButtonState.show(buttonState),
      );
    },
  );

  glfwSetFramebufferSizeCallback(
    primaryWindow,
    (window, w, h) => {
      //glViewport(0, 0, w, h);
      print_endline(
        "Framebuffer size changed: "
        ++ string_of_int(w)
        ++ ", "
        ++ string_of_int(h),
      );
      width := w;
      height := h;
      render(window);
    },
  );

  glfwSetWindowSizeCallback(
    primaryWindow,
    (window, w, h) => {
      print_endline(
        "Window size changed: "
        ++ string_of_int(w)
        ++ ", "
        ++ string_of_int(h),
      );
      width := w;
      height := h;
      render(window);
    },
  );
  
  glfwSetWindowPosCallback(
    primaryWindow,
    (_w, x, y) => {
      print_endline(
        "Window position changed: "
        ++ string_of_int(x)
        ++ ", "
        ++ string_of_int(y),
      );
    },
  );

  /* glfwMaximizeWindow(w); */

  let nativeWindow = glfwGetNativeWindow(primaryWindow);
  print_endline ("Native window handle/pointer: " ++ string_of_int(Obj.magic(nativeWindow)));

  let frame = ref(0);
  glfwRenderLoop(_t => {
    render(primaryWindow);
    /* render(secondaryWindow); */

    let s = Glfw.glfwGetClipboardString(primaryWindow);

    let v = switch(s) {
    | Some(v) => "Some(" ++ v ++ ")"
    | None => "None"
    }
    print_endline ("Clipboard string before: " ++ v);

    Glfw.glfwSetClipboardString(primaryWindow, "test clipboard: " ++ string_of_int(frame^));
    
    let s = Glfw.glfwGetClipboardString(primaryWindow);
    let v = switch(s) {
    | Some(v) => "Some(" ++ v ++ ")"
    | None => "None"
    }
    print_endline ("Clipboard string after: " ++ v);


    /* Run the GC so we can catch any GC-related crashes early! */
    Gc.full_major();

    glfwPollEvents();
    glfwWindowShouldClose(primaryWindow);
  });

  print_endline("Done!");
  glfwTerminate();
};

module Key = Glfw_key;

include (module type of Glfw_types);

let glfwInit: unit => bool;
let glfwCreateWindow:
  (int, int, ~sharedContext: Window.t=?, string) => Window.t;
let glfwMakeContextCurrent: Window.t => unit;
let glfwWindowShouldClose: Window.t => bool;
let glfwPollEvents: unit => unit;
let glfwTerminate: unit => unit;
let glfwSwapBuffers: Window.t => unit;
let glfwSetWindowPos: (Window.t, int, int) => unit;
let glfwSetWindowSize: (Window.t, int, int) => unit;
let glfwGetFramebufferSize: Window.t => Window.frameBufferSize;
let glfwGetWindowSize: Window.t => Window.windowSize;
let glfwMaximizeWindow: Window.t => unit;
let glfwSetWindowTitle: (Window.t, string) => unit;
let glfwShowWindow: Window.t => unit;
let glfwHideWindow: Window.t => unit;
let glfwDestroyWindow: Window.t => unit;
let glfwSwapInterval: int => unit;
let glfwGetTime: unit => float;
let glfwSetTime: float => unit;
let glfwSetWindowIcon: (Window.t, string) => unit;

let glfwGetClipboardString: (Window.t) => option(string);
let glfwSetClipboardString: (Window.t, string) => unit;

let glfwGetNativeWindow: Window.t => NativeWindow.t;

module Modifier: {
  type t;

  let of_int: int => t;

  let isShiftPressed: t => bool;
  let isControlPressed: t => bool;
  let isAltPressed: t => bool;
  let isSuperPressed: t => bool;

  let show: t => string;
};

module MouseButton: {
  type t =
    | GLFW_MOUSE_LEFT
    | GLFW_MOUSE_RIGHT
    | GLFW_MOUSE_MIDDLE
    | GLFW_MOUSE_BUTTON_4
    | GLFW_MOUSE_BUTTON_5
    | GLFW_MOUSE_BUTTON_6
    | GLFW_MOUSE_BUTTON_7
    | GLFW_MOUSE_BUTTON_8;

  let show: t => string;
};

module Monitor: {
  type t;

  type position = {
    x: int,
    y: int,
  };

  type dimensions = {
    width: int,
    height: int,
  };
};

module VideoMode: {
  type t = {
    width: int,
    height: int,
  };
};

let glfwGetPrimaryMonitor: unit => Monitor.t;
let glfwGetVideoMode: Monitor.t => VideoMode.t;
let glfwGetMonitorPos: Monitor.t => Monitor.position;
let glfwGetMonitorPhysicalSize: Monitor.t => Monitor.dimensions;

type windowHint =
  | GLFW_RESIZABLE
  | GLFW_VISIBLE
  | GLFW_DECORATED
  | GLFW_FOCUSED
  | GLFW_AUTO_ICONIFY
  | GLFW_FLOATING
  | GLFW_MAXIMIZED;

module ButtonState: {
  type t =
    | GLFW_PRESS
    | GLFW_RELEASE
    | GLFW_REPEAT;

  let show: t => string;
};

let glfwDefaultWindowHints: unit => unit;
let glfwWindowHint: (windowHint, bool) => unit;

type glfwCursorPosCallback = (Window.t, float, float) => unit;
let glfwSetCursorPosCallback: (Window.t, glfwCursorPosCallback) => unit;

type glfwCharCallback = (Window.t, int) => unit;
let glfwSetCharCallback: (Window.t, glfwCharCallback) => unit;

type glfwCharModsCallback = (Window.t, int, Modifier.t) => unit;
let glfwSetCharModsCallback: (Window.t, glfwCharModsCallback) => unit;

type glfwKeyCallback =
  (Window.t, Key.t, int, ButtonState.t, Modifier.t) => unit;
let glfwSetKeyCallback: (Window.t, glfwKeyCallback) => unit;

type glfwScrollCallback = (Window.t, float, float) => unit;
let glfwSetScrollCallback: (Window.t, glfwScrollCallback) => unit;

type glfwMouseButtonCallback =
  (Window.t, MouseButton.t, ButtonState.t, Modifier.t) => unit;
let glfwSetMouseButtonCallback: (Window.t, glfwMouseButtonCallback) => unit;

let glfwSetFramebufferSizeCallback:
  (Window.t, glfwFramebufferSizeCallback) => unit;

let glfwSetWindowSizeCallback: (Window.t, glfwWindowSizeCallback) => unit;
let glfwSetWindowPosCallback: (Window.t, glfwWindowPosCallback) => unit;

type glfwCursorPos = {
  mouseX: float,
  mouseY: float,
};
let glfwGetCursorPos: Window.t => glfwCursorPos;

type glfwCursor;
type glfwCursorShape =
  | GLFW_ARROW_CURSOR
  | GLFW_IBEAM_CURSOR
  | GLFW_CROSSHAIR_CURSOR
  | GLFW_HAND_CURSOR
  | GLFW_HRESIZE_CURSOR
  | GLFW_VRESIZE_CURSOR;
let glfwCreateStandardCursor: glfwCursorShape => glfwCursor;
let glfwDestroyCursor: glfwCursor => unit;
let glfwSetCursor: (Window.t, glfwCursor) => unit;

type glfwRenderLoopCallback = float => bool;

let glfwRenderLoop: glfwRenderLoopCallback => unit;

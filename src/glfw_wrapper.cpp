#include <stdio.h>

#include <caml/mlvalues.h>
#include <caml/bigarray.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include <caml/threads.h>

#include <glad/glad.h>

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

#include "stb_image.h"

// Include native GLFW access functions
// Documentation here: https://www.glfw.org/docs/latest/group__native.html
#ifdef _WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <winuser.h>
#elif __APPLE__
    #define GLFW_EXPOSE_NATIVE_COCOA
#else
    #define GLFW_EXPOSE_NATIVE_X11
#endif
#include <GLFW/glfw3native.h>

#include <reglfw_image.h>

extern "C" {

#define Val_none Val_int(0)
#define Some_val(v) Field(v,0)

    struct WindowInfo {
        GLFWwindow* pWindow;
        bool isDestroyed;
        value vSetFramebufferSizeCallback;
        value vSetWindowSizeCallback;
        value vSetWindowPosCallback;
        value vSetCursorPosCallback;
        value vKeyCallback;
        value vMouseButtonCallback;
        value vCharCallback;
        value vCharModsCallback;
        value vScrollCallback;
    };

    static WindowInfo* sActiveWindows[255];
    static int sActiveWindowCount = 0;

    int variantToWindowHint(value vVal) {
        switch (Int_val(vVal)) {
            case 0:
                return GLFW_RESIZABLE;
            case 1:
                return GLFW_VISIBLE;
            case 2:
                return GLFW_DECORATED;
            case 3:
                return GLFW_FOCUSED;
            case 4:
                return GLFW_AUTO_ICONIFY;
            case 5:
                return GLFW_FLOATING;
            case 6:
                return GLFW_MAXIMIZED;
            default:
                printf("Unexpected window hint type.\n");
                return 0;
        }
    }

    value buttonStateToVariant(int state) {
        switch(state) {
        case GLFW_PRESS:
            return Val_int(0);
        case GLFW_RELEASE:
            return Val_int(1);
        case GLFW_REPEAT:
            return Val_int(2);
        default:
            printf("Unexpected button state");
            return Val_int(0);
        }
    }

    int variantToCursorShape(value shape) {
      switch(Int_val(shape)) {
      case 0:
        return GLFW_ARROW_CURSOR;
      case 1:
        return GLFW_IBEAM_CURSOR;
      case 2:
        return GLFW_CROSSHAIR_CURSOR;
      case 3:
        return GLFW_HAND_CURSOR;
      case 4:
        return GLFW_HRESIZE_CURSOR;
      case 5:
        return GLFW_VRESIZE_CURSOR;
      default:
        printf("Unexpected cursor shape.\n");
        return 0;
      }
    }

    WindowInfo* getWindowInfoFromWindow(GLFWwindow *w) {
        WindowInfo *pInfo;
        for (int i = 0; i < sActiveWindowCount; i++) {
            if (sActiveWindows[i] && sActiveWindows[i]->pWindow && sActiveWindows[i]->pWindow == w) {
                pInfo = sActiveWindows[i];
            }
        }
        return pInfo;
    }

    CAMLprim value
    caml_test_callback_success(value vSuccess, value vFailure) {
        CAMLparam2(vSuccess, vFailure);

        caml_callback(vSuccess, Val_int(900));
        CAMLreturn(Val_unit);
    }

    CAMLprim value
    caml_glfwJavascriptRenderLoop(value unit) {
        // no-op for native
        return Val_unit;
    }

    CAMLprim value
    caml_test_callback_failure(value vSuccess, value vFailure) {
        CAMLparam2(vSuccess, vFailure);

        caml_callback(vFailure, caml_copy_string("failure!"));
        CAMLreturn(Val_unit);
    }

    CAMLprim value
    caml_glfwInit(value unit)
    {
        int ret = glfwInit();
        return Val_bool(ret);
    }

    void framebuffer_size_callback(GLFWwindow *pWin, int iWidth, int iHeight) {
        // Is there a window info?
        WindowInfo * pWinInfo = getWindowInfoFromWindow(pWin);

        if (pWinInfo && pWinInfo->vSetFramebufferSizeCallback != Val_unit) {
            (void) caml_callback3((value)pWinInfo->vSetFramebufferSizeCallback, ((value)(void *)pWinInfo), Val_int(iWidth), Val_int(iHeight));
        }
    }

    void window_size_callback(GLFWwindow *pWin, int iWidth, int iHeight) {
        // Is there a window info?
        WindowInfo * pWinInfo = getWindowInfoFromWindow(pWin);

        if (pWinInfo && pWinInfo->vSetWindowSizeCallback != Val_unit) {
            (void) caml_callback3((value)pWinInfo->vSetWindowSizeCallback, ((value)(void *)pWinInfo), Val_int(iWidth), Val_int(iHeight));
        }
    }

    void window_pos_callback(GLFWwindow *pWin, int iX, int iY) {
        // Is there a window info?
        WindowInfo * pWinInfo = getWindowInfoFromWindow(pWin);

        if (pWinInfo && pWinInfo->vSetWindowPosCallback != Val_unit) {
            (void) caml_callback3((value)pWinInfo->vSetWindowPosCallback, ((value)(void *)pWinInfo), Val_int(iX), Val_int(iY));
        }
    }

    void key_callback(GLFWwindow *pWin, int key, int scancode, int action, int mods) {
        WindowInfo * pWinInfo = getWindowInfoFromWindow(pWin);
        if (pWinInfo && pWinInfo->vKeyCallback != Val_unit) {
            value* pArgs = (value *)malloc(sizeof(value) * 5);
            pArgs[0] = (value)pWinInfo;
            pArgs[1] = Val_int(key);
            pArgs[2] = Val_int(scancode);
            pArgs[3] = buttonStateToVariant(action);
            pArgs[4] = Val_int(mods);

            (void) caml_callbackN((value)pWinInfo->vKeyCallback, 5, pArgs);
            free(pArgs);
        }
    }

    void mouse_button_callback(GLFWwindow *pWin, int mouseButton, int action, int mods) {
        WindowInfo * pWinInfo = getWindowInfoFromWindow(pWin);
        if (pWinInfo && pWinInfo->vMouseButtonCallback != Val_unit) {
            value* pArgs = (value *)malloc(sizeof(value) * 4);
            pArgs[0] = (value)pWinInfo;
            pArgs[1] = Val_int(mouseButton);
            pArgs[2] = buttonStateToVariant(action);
            pArgs[3] = Val_int(mods);

            (void) caml_callbackN((value)pWinInfo->vMouseButtonCallback, 4, pArgs);
            free(pArgs);
        }
    }

    void scroll_callback(GLFWwindow *pWin, double dblWidth, double dblHeight) {
        // Is there a window info?
        WindowInfo * pWinInfo = getWindowInfoFromWindow(pWin);

        if (pWinInfo && pWinInfo->vScrollCallback != Val_unit) {
            (void) caml_callback3((value)pWinInfo->vScrollCallback, ((value)(void *)pWinInfo), caml_copy_double(dblWidth), caml_copy_double(dblHeight));
        }
    }

    void cursor_pos_callback(GLFWwindow *pWin, double xPos, double yPos) {
        // Is there a window info?
        WindowInfo * pWinInfo = getWindowInfoFromWindow(pWin);

        if (pWinInfo && pWinInfo->vSetCursorPosCallback != Val_unit) {
            (void) caml_callback3((value)pWinInfo->vSetCursorPosCallback, ((value)(void *)pWinInfo), caml_copy_double(xPos), caml_copy_double(yPos));
        }
    }

    void char_callback(GLFWwindow *pWin, unsigned int codepoint) {
        WindowInfo *pWinInfo = getWindowInfoFromWindow(pWin);

        if (pWinInfo && pWinInfo->vCharCallback != Val_unit) {
            (void) caml_callback2((value)pWinInfo->vCharCallback, ((value)(void *)pWinInfo), Val_int(codepoint));
        }
    }

    void char_mods_callback(GLFWwindow *pWin, unsigned int codepoint, int modifier) {
        WindowInfo *pWinInfo = getWindowInfoFromWindow(pWin);

        if (pWinInfo && pWinInfo->vCharModsCallback != Val_unit) {
            (void) caml_callback3((value)pWinInfo->vCharModsCallback, ((value)(void *)pWinInfo), Val_int(codepoint), Val_int(modifier));
        }
    }

    CAMLprim value
    caml_glfwGetClipboardString(value vWindow) {
        CAMLparam1(vWindow);
        CAMLlocal1(ret);
        
        WindowInfo *pWinInfo = (WindowInfo *)vWindow;
        GLFWwindow *wd = pWinInfo->pWindow;

        const char* sz = glfwGetClipboardString(wd);

        if (sz == NULL) {
            ret = Val_none;
        } else {
            ret = caml_alloc(1, 0);
            Store_field(ret, 0, caml_copy_string(sz));
        }

        CAMLreturn(ret);
    }

    CAMLprim value
    caml_glfwSetClipboardString(value vWindow, value vSz) {
        CAMLparam2(vWindow, vSz);

        char *s = String_val(vSz);
        WindowInfo *pWinInfo = (WindowInfo *)vWindow;
        GLFWwindow *wd = pWinInfo->pWindow;

        glfwSetClipboardString(wd, s);
        CAMLreturn(Val_unit);
    }

    CAMLprim value
    caml_glfwGetNativeWindow(value vWindow) {
        WindowInfo *pWinInfo = (WindowInfo *)vWindow;

#ifdef _WIN32
        HWND hwnd = glfwGetWin32Window(pWinInfo->pWindow);
        return (value)hwnd;
#elif __APPLE__
       void* pWin = glfwGetCocoaWindow(pWinInfo->pWindow);
       return (value)pWin;
#else
       long unsigned int lWin = glfwGetX11Window(pWinInfo->pWindow);
       return (value)lWin;
#endif
    }

    CAMLprim value
    caml_glfwCreateWindow(value iWidth, value iHeight, value vSharedContext, value sTitle)
    {
      CAMLparam4(iWidth, iHeight, vSharedContext, sTitle);

      GLFWwindow* wd;           /* window desciptor/handle */
      int w = Int_val(iWidth);
      int h = Int_val(iHeight);
      char *s = String_val(sTitle);

      /* 
      vSharedContext is an optional labeled argument in OCaml
      Depending on the value of vSharedContext we create a normal window if is none
      or a window with shared context https://www.glfw.org/docs/latest/context_guide.html#context_sharing if the value exists
      Using this binding as an example: https://www.linux-nantes.org/~fmonnier/OCaml/ocaml-wrapping-c.html#ref_optlabel
      */

      // It might be none, we check that against the Val_none macro defined at the top
      if (vSharedContext == Val_none) {
        wd = glfwCreateWindow(w, h, s, NULL, NULL);
      } else {
        // It might be a real value, so we get it by using the Some_val macro and cast it to a WindowInfo
        WindowInfo* sharedWindowInfo = (WindowInfo *)Some_val(vSharedContext);
        GLFWwindow* sharedWindow = sharedWindowInfo->pWindow;
        wd = glfwCreateWindow(w, h, s, NULL, sharedWindow);
      };

      struct WindowInfo* pWindowInfo = (WindowInfo *)malloc(sizeof(WindowInfo));
      pWindowInfo->pWindow = wd;
      pWindowInfo->isDestroyed = false;
      pWindowInfo->vSetFramebufferSizeCallback = Val_unit;
      pWindowInfo->vSetWindowSizeCallback = Val_unit;
      pWindowInfo->vSetWindowPosCallback = Val_unit;
      pWindowInfo->vSetCursorPosCallback = Val_unit;
      pWindowInfo->vCharCallback = Val_unit;
      pWindowInfo->vCharModsCallback = Val_unit;
      pWindowInfo->vKeyCallback = Val_unit;
      pWindowInfo->vMouseButtonCallback = Val_unit;
      pWindowInfo->vScrollCallback = Val_unit;

      glfwSetFramebufferSizeCallback(wd, framebuffer_size_callback);
      glfwSetWindowSizeCallback(wd, window_size_callback);
      glfwSetWindowPosCallback(wd, window_pos_callback);
      glfwSetCursorPosCallback(wd, cursor_pos_callback);
      glfwSetCharCallback(wd, char_callback);
      glfwSetCharModsCallback(wd, char_mods_callback);
      glfwSetKeyCallback(wd, key_callback);
      glfwSetMouseButtonCallback(wd, mouse_button_callback);
      glfwSetScrollCallback(wd, scroll_callback);

      sActiveWindows[sActiveWindowCount] = pWindowInfo;
      sActiveWindowCount++;

      CAMLreturn((value)pWindowInfo);
    }

    CAMLprim value
    caml_glfwSwapInterval(value vInterval) {
        glfwSwapInterval(Int_val(vInterval));
        return Val_unit;
    }

    CAMLprim value
    caml_glfwSetWindowTitle(value vWindow, value vTitle) {
        WindowInfo* pWindowInfo = (WindowInfo *)vWindow;
        char *szTitle = String_val(vTitle);
        glfwSetWindowTitle(pWindowInfo->pWindow, szTitle);
        return Val_unit;
    }

    CAMLprim value
    caml_glfwDefaultWindowHints() {
        glfwDefaultWindowHints();
        return Val_unit;
    }

    CAMLprim value
    caml_glfwWindowHint(value vHint, value vVal) {
        int windowHint = variantToWindowHint(vHint);
        int val = Bool_val(vVal) ? GLFW_TRUE : GLFW_FALSE;
        glfwWindowHint(windowHint, val);
        return Val_unit;
    }

    CAMLprim value
    caml_glfwSetWindowPos(value vWindow, value vX, value vY) {
        WindowInfo* pWindowInfo = (WindowInfo *)vWindow;
        glfwSetWindowPos(pWindowInfo->pWindow, Int_val(vX), Int_val(vY));
        return Val_unit;
    }

    CAMLprim value
    caml_glfwSetWindowSize(value vWindow, value vWidth, value vHeight) {
        WindowInfo* pWindowInfo = (WindowInfo *)vWindow;
        glfwSetWindowSize(pWindowInfo->pWindow, Int_val(vWidth), Int_val(vHeight));
        return Val_unit;
    }

    CAMLprim value
    caml_glfwMaximizeWindow(value vWindow) {
        WindowInfo *pWindowInfo = (WindowInfo *)vWindow;
        glfwMaximizeWindow(pWindowInfo->pWindow);
        return Val_unit;
    }

    CAMLprim value
    caml_glfwMakeContextCurrent(value window)
    {
        WindowInfo* wd = (WindowInfo *)window;
        glfwMakeContextCurrent(wd->pWindow);
        gladLoadGLES2Loader((GLADloadproc) glfwGetProcAddress);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        //glViewport(0, 0, 640, 800);
        //glClearColor(1, 1, 1, 1);
        glClearStencil(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        return Val_unit;
    }

    CAMLprim value
    caml_glfwSetWindowSizeCallback(value vWindow, value vCallback) {
        CAMLparam2(vWindow, vCallback);

        WindowInfo *pWinInfo = (WindowInfo *)vWindow;

        if (pWinInfo) {
            // TODO: Recycle existing callback if any!

            // We need to mark the closure as being a global root, so the garbage
            // collector knows it is being used.
            pWinInfo->vSetWindowSizeCallback = vCallback;
            caml_register_global_root(&(pWinInfo->vSetWindowSizeCallback));
        }

        CAMLreturn(Val_unit);
    }

    CAMLprim value
    caml_glfwSetWindowPosCallback(value vWindow, value vCallback) {
        CAMLparam2(vWindow, vCallback);

        WindowInfo *pWinInfo = (WindowInfo *)vWindow;

        if (pWinInfo) {
            // TODO: Recycle existing callback if any!

            // We need to mark the closure as being a global root, so the garbage
            // collector knows it is being used.
            pWinInfo->vSetWindowPosCallback = vCallback;
            caml_register_global_root(&(pWinInfo->vSetWindowPosCallback));
        }

        CAMLreturn(Val_unit);
    }

    CAMLprim value
    caml_glfwSetFramebufferSizeCallback(value vWindow, value vCallback) {
        CAMLparam2(vWindow, vCallback);

        WindowInfo *pWinInfo = (WindowInfo *)vWindow;

        if (pWinInfo) {
            // TODO: Recycle existing callback if any!

            // We need to mark the closure as being a global root, so the garbage
            // collector knows it is being used.
            pWinInfo->vSetFramebufferSizeCallback = vCallback;
            caml_register_global_root(&(pWinInfo->vSetFramebufferSizeCallback));
        }

        CAMLreturn(Val_unit);
    }

    CAMLprim value
    caml_glfwSetCharCallback(value vWindow, value vCallback) {
        CAMLparam2(vWindow, vCallback);

        WindowInfo *pWinInfo = (WindowInfo *)vWindow;

        if (pWinInfo) {
            // TODO: Recycle existing callback if any!

            // We need to mark the closure as being a global root, so the garbage
            // collector knows it is being used.
            pWinInfo->vCharCallback = vCallback;
            caml_register_global_root(&(pWinInfo->vCharCallback));
        }

        CAMLreturn(Val_unit);
    }

    CAMLprim value
    caml_glfwSetCharModsCallback(value vWindow, value vCallback) {
        CAMLparam2(vWindow, vCallback);

        WindowInfo *pWinInfo = (WindowInfo *)vWindow;

        if (pWinInfo) {
            // TODO: Recycle existing callback if any!

            // We need to mark the closure as being a global root, so the garbage
            // collector knows it is being used.
            pWinInfo->vCharModsCallback = vCallback;
            caml_register_global_root(&(pWinInfo->vCharModsCallback));
        }

        CAMLreturn(Val_unit);
    }

    CAMLprim value
    caml_glfwSetKeyCallback(value vWindow, value vCallback) {
        CAMLparam2(vWindow, vCallback);

        WindowInfo *pWinInfo = (WindowInfo *)vWindow;

        if (pWinInfo) {
            // TODO: Recycle existing callback if any!

            // We need to mark the closure as being a global root, so the garbage
            // collector knows it is being used.
            pWinInfo->vKeyCallback = vCallback;
            caml_register_global_root(&(pWinInfo->vKeyCallback));
        }

        CAMLreturn(Val_unit);
    }

    CAMLprim value
    caml_glfwSetMouseButtonCallback(value vWindow, value vCallback) {
        CAMLparam2(vWindow, vCallback);

        WindowInfo *pWinInfo = (WindowInfo *)vWindow;

        if (pWinInfo) {
            // TODO: Recycle existing callback if any!

            // We need to mark the closure as being a global root, so the garbage
            // collector knows it is being used.
            pWinInfo->vMouseButtonCallback = vCallback;
            caml_register_global_root(&(pWinInfo->vMouseButtonCallback));
        }

        CAMLreturn(Val_unit);
    }

    CAMLprim value
    caml_glfwSetScrollCallback(value vWindow, value vCallback) {
        CAMLparam2(vWindow, vCallback);

        WindowInfo *pWinInfo = (WindowInfo *)vWindow;

        if (pWinInfo) {
            // TODO: Recycle existing callback if any!

            // We need to mark the closure as being a global root, so the garbage
            // collector knows it is being used.
            pWinInfo->vScrollCallback = vCallback;
            caml_register_global_root(&(pWinInfo->vScrollCallback));
        }

        CAMLreturn(Val_unit);
    }

    double
    caml_glfwGetTime() {
        return glfwGetTime();
    }

    CAMLprim value
    caml_glfwGetTime_byte() {
        return caml_copy_double(caml_glfwGetTime());
    }

    void
    caml_glfwSetTime(double t) {
        glfwSetTime(t);
    }

    CAMLprim value
    caml_glfwSetTime_byte(value vTime) {
        glfwSetTime(Double_val(vTime));
        return Val_unit;
    }

    CAMLprim value
    caml_glfwGetPrimaryMonitor()
    {
        return (value)glfwGetPrimaryMonitor();
    }

    CAMLprim value
    caml_glfwGetVideoMode(value vMonitor)
    {
        CAMLparam1(vMonitor);
        CAMLlocal1(ret);
        GLFWmonitor* pMonitor = (GLFWmonitor*)vMonitor;
        const GLFWvidmode* pVidMode = glfwGetVideoMode(pMonitor);

        ret = caml_alloc(2, 0);
        Store_field(ret, 0, Val_int(pVidMode->width));
        Store_field(ret, 1, Val_int(pVidMode->height));

        CAMLreturn(ret);
    }

    CAMLprim value
    caml_glfwGetFramebufferSize(value vWindow)
    {
        CAMLparam1(vWindow);
        CAMLlocal1(ret);
        WindowInfo* pWindowInfo = (WindowInfo *)vWindow;

        int width, height;
        glfwGetFramebufferSize(pWindowInfo->pWindow, &width, &height);

        ret = caml_alloc(2, 0);
        Store_field(ret, 0, Val_int(width));
        Store_field(ret, 1, Val_int(height));

        CAMLreturn(ret);
    }

    CAMLprim value
    caml_glfwGetWindowSize(value vWindow)
    {
        CAMLparam1(vWindow);
        CAMLlocal1(ret);
        WindowInfo* pWindowInfo = (WindowInfo *)vWindow;

        int width, height;
        glfwGetWindowSize(pWindowInfo->pWindow, &width, &height);

        ret = caml_alloc(2, 0);
        Store_field(ret, 0, Val_int(width));
        Store_field(ret, 1, Val_int(height));

        CAMLreturn(ret);
    }

    CAMLprim value
    caml_glfwGetMonitorPos(value vMonitor) {
        CAMLparam1(vMonitor);
        CAMLlocal1(ret);
        GLFWmonitor* pMonitor = (GLFWmonitor*)vMonitor;

        int xPos, yPos;
        glfwGetMonitorPos(pMonitor, &xPos, &yPos);

        ret = caml_alloc(2, 0);
        Store_field(ret, 0, Val_int(xPos));
        Store_field(ret, 1, Val_int(yPos));

        CAMLreturn(ret);
    }

    CAMLprim value
    caml_glfwGetMonitorPhysicalSize(value vMonitor)
    {
        CAMLparam1(vMonitor);
        CAMLlocal1(ret);
        GLFWmonitor* zMonitor = (GLFWmonitor*)vMonitor;

        int widthMM, heightMM;
        glfwGetMonitorPhysicalSize(zMonitor, &widthMM, &heightMM);

        ret = caml_alloc(2, 0);
        Store_field(ret, 0, Val_int(widthMM));
        Store_field(ret, 1, Val_int(heightMM));

        CAMLreturn(ret);
    }

    CAMLprim value
    caml_glfwSetCursorPosCallback(value vWindow, value vCallback) {
        CAMLparam2(vWindow, vCallback);

        WindowInfo *pWinInfo = (WindowInfo *)vWindow;

        if (pWinInfo) {
            // TODO: Recycle existing callback if any!

            // We need to mark the closure as being a global root, so the garbage
            // collector knows it is being used.
            pWinInfo->vSetCursorPosCallback = vCallback;
            caml_register_global_root(&(pWinInfo->vSetCursorPosCallback));
        }

        CAMLreturn(Val_unit);
    }

    CAMLprim value
    caml_glfwGetCursorPos(value vWindow) {
        CAMLparam1(vWindow);
        CAMLlocal1(ret);

        WindowInfo *pWinInfo = (WindowInfo *)vWindow;

        double xpos, ypos;
        glfwGetCursorPos(pWinInfo->pWindow, &xpos, &ypos);

        ret = caml_alloc(2 * Double_wosize, Double_array_tag);
        Store_double_field(ret, 0, xpos);
        Store_double_field(ret, 1, ypos);
        CAMLreturn(ret);
    }

    CAMLprim value
    caml_glfwCreateStandardCursor(value shape) {
      CAMLparam1(shape);
      GLFWcursor *cursor;
      cursor = glfwCreateStandardCursor(variantToCursorShape(shape));
      if (cursor == NULL) { // If there was an error
        caml_failwith("Error thrown while creating GLFW cursor. Make sure to init GLFW before creating a cursor");
      } else {
        CAMLreturn((value) cursor);
      }
    }

    CAMLprim value
    caml_glfwDestroyCursor(value cursor) {
      CAMLparam1(cursor);
      GLFWcursor *cd = (GLFWcursor *) cursor;
      glfwDestroyCursor(cd);
      CAMLreturn(Val_unit);
    }

    CAMLprim value
    caml_glfwSetCursor(value window, value cursor) {
      CAMLparam2(window, cursor);
      WindowInfo *wd = (WindowInfo *) window;
      GLFWcursor *cd = (GLFWcursor *) cursor;
      glfwSetCursor(wd->pWindow, cd);
      CAMLreturn(Val_unit);
    }

    CAMLprim value
    caml_glfwSetWindowIcon(value vWindow, value vPath) {
        WindowInfo* pWindowInfo = (WindowInfo *)vWindow;

        int numberOfImages = 1;
        GLFWimage images[1];
        char *szPath = String_val(vPath);

        int channels;
        int width;
        int height;
        images[0].pixels = stbi_load(szPath, &images[0].width, &images[0].height, 0, 4);
        glfwSetWindowIcon(pWindowInfo->pWindow, 1, images);

        return Val_unit;
    }

    CAMLprim value
    caml_printFrameBufferSize(value window)
    {
        WindowInfo* wd = (WindowInfo*)window;
        int fbwidth;
        int fbheight;
        glfwGetFramebufferSize(wd->pWindow, &fbwidth, &fbheight);
        return Val_unit;
    }

    CAMLprim value
    caml_glfwWindowShouldClose(value window)
    {
        WindowInfo *wd = (WindowInfo *)window;

        if (wd->isDestroyed) {
            return Val_true;
        } else {
            int val = glfwWindowShouldClose(wd->pWindow);
            return Val_bool(val);
        }
    }

    CAMLprim value
    caml_glfwShowWindow(value vWindow) {
        WindowInfo *wd = (WindowInfo *)vWindow;
        glfwShowWindow(wd->pWindow);
        return Val_unit;
    }

    CAMLprim value
    caml_glfwDestroyWindow(value vWindow) {
        WindowInfo* wd = (WindowInfo*)vWindow;
        if (!wd->isDestroyed) {
          wd->isDestroyed = true;
          glfwDestroyWindow(wd->pWindow);
        }

        return Val_unit;
    }

    CAMLprim value
    caml_glfwHideWindow(value vWindow) {
        WindowInfo *wd = (WindowInfo *)vWindow;
        glfwHideWindow(wd->pWindow);
        return Val_unit;
    }

    CAMLprim value
    caml_glfwPollEvents(value unit)
    {
        glfwPollEvents();
        return Val_unit;
    }

    CAMLprim value
    caml_glfwSwapBuffers(value window)
    {
        WindowInfo *wd = (WindowInfo*)window;
        GLFWwindow *pWindow = wd->pWindow;
        caml_release_runtime_system();
        glfwSwapBuffers(pWindow);
        caml_acquire_runtime_system();
        return Val_unit;
    }

    CAMLprim value
    caml_glfwTerminate(value unit)
    {
        glfwTerminate();
        return Val_unit;
    }
}

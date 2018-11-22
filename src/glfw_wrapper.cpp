#include <stdio.h>

#include <caml/mlvalues.h>
#include <caml/bigarray.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/callback.h>

#include <glad/glad.h>

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

#include <reglfw_image.h>

extern "C" {

    struct WindowInfo {
        GLFWwindow* pWindow;
        value vSetFramebufferSizeCallback;
        value vSetCursorPosCallback;
        value vKeyCallback;
        value vCharCallback;
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

    WindowInfo* getWindowInfoFromWindow(GLFWwindow *w) {
        WindowInfo *pInfo;
        for (int i = 0; i < sActiveWindowCount; i++) {
            if (sActiveWindows[i] && sActiveWindows[i]->pWindow) {
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

    CAMLprim value
    caml_glfwCreateWindow(value iWidth, value iHeight, value sTitle)
    {
      CAMLparam3(iWidth, iHeight, sTitle);

      GLFWwindow* wd;           /* window desciptor/handle */
      int w = Int_val(iWidth);
      int h = Int_val(iHeight);
      char *s = String_val(sTitle);

      wd = glfwCreateWindow(w, h, s,
                            NULL, NULL);

      struct WindowInfo* pWindowInfo = (WindowInfo *)malloc(sizeof(WindowInfo));
      pWindowInfo->pWindow = wd;
      pWindowInfo->vSetFramebufferSizeCallback = Val_unit;
      pWindowInfo->vSetCursorPosCallback = Val_unit;
      pWindowInfo->vCharCallback = Val_unit;
      pWindowInfo->vKeyCallback = Val_unit;
      pWindowInfo->vScrollCallback = Val_unit;

      glfwSetFramebufferSizeCallback(wd, framebuffer_size_callback);
      glfwSetCursorPosCallback(wd, cursor_pos_callback);
      glfwSetCharCallback(wd, char_callback);
      glfwSetKeyCallback(wd, key_callback);
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
        return Val_unit;
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
        int val = glfwWindowShouldClose(wd->pWindow);
        return Val_bool(val);
    }

    CAMLprim value
    caml_glfwShowWindow(value vWindow) {
        WindowInfo *wd = (WindowInfo *)vWindow;
        glfwShowWindow(wd->pWindow);
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
        glfwSwapBuffers(wd->pWindow);
        return Val_unit;
    }

    CAMLprim value
    caml_glfwTerminate(value unit)
    {
        glfwTerminate();
        return Val_unit;
    }
}

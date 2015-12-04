package slib.platform.android.ui.view;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import slib.platform.android.Logger;
import android.annotation.SuppressLint;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;

public class UiGLView extends GLSurfaceView implements GLSurfaceView.Renderer {
	
	public static UiGLView _create(Context context) {
		try {
			UiGLView ret = new UiGLView(context);
			return ret;
		} catch (Exception e) {
			Logger.exception(e);
		}
		return null;
	}
	
	public static boolean _setRenderMode(View view, int mode) {
		if (view instanceof GLSurfaceView) {
			if (mode == 0) {
				((GLSurfaceView)view).setRenderMode(UiGLView.RENDERMODE_CONTINUOUSLY);				
			} else {
				((GLSurfaceView)view).setRenderMode(UiGLView.RENDERMODE_WHEN_DIRTY);				
			}
			return true;
		}
		return false;
	}

	public static void _requestRender(View view) {
		if (view instanceof UiGLView) {			
			((UiGLView)view).requestRender();				
		}
	}

	private static native void nativeOnCreate(long instance);
	public static void onEventCreate(UiGLView view) {
		long instance = UiView.getInstance(view);
		if (instance != 0) {
			nativeOnCreate(instance);
		}
	}
	
	private static native void nativeOnFrame(long instance, int width, int height);
	public static void onEventFrame(UiGLView view, int width, int height) {
		long instance = UiView.getInstance(view);
		if (instance != 0) {
			nativeOnFrame(instance, width, height);
		}
	}
	
	public UiGLView(Context context) {
		super(context);
		
		setEGLContextClientVersion(2);
		setEGLConfigChooser(new MultisampleConfigChooser());
		
		setRenderer(this);
		setRenderMode(RENDERMODE_WHEN_DIRTY);
	}
	
	public boolean onKeyDown(int keycode, KeyEvent event) {
		if (UiView.onEventKey(this, true, keycode, event)) {
			return super.onKeyDown(keycode, event);
		}
		return false;
	}
	
	public boolean onKeyUp(int keycode, KeyEvent event) {
		if (UiView.onEventKey(this, false, keycode, event)) {
			return super.onKeyUp(keycode, event);
		}
		return false;
	}
	
	@SuppressLint("ClickableViewAccessibility")
	public boolean onTouchEvent(MotionEvent event) {		
		if (UiView.onEventTouch(this, event)) {
			return true;
		}
		return false;
	}

	
	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		onEventCreate(this);
	}

	@Override
	public void onSurfaceChanged(GL10 gl, int width, int height) {
		widthViewport = width;
		heightViewport = height;
	}

	int widthViewport = 0;
	int heightViewport = 0;
	@Override
	public void onDrawFrame(GL10 gl) {
		onEventFrame(this, widthViewport, heightViewport);
	}
}
package com.roaraudio.muroard;

public class NativeUtils {
    static {
        System.loadLibrary("muroard");
    }
    
    /**
     * Die Methode liefert Info über CPU
     */
//    public native String getCpuInfo();
    public native int startMuroarD(String argv, int argc);
    public native void stopMuroarD();

}

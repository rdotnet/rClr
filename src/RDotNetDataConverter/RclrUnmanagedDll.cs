using System;
using RDotNet.NativeLibrary;
using System.Runtime.InteropServices;
using System.Reflection;
using System.IO;
using DynamicInterop;

namespace Rclr
{
    public class RclrUnmanagedDll : IUnmanagedDll 
    {
        public RclrUnmanagedDll(string dllName)
        {
            if (!File.Exists(dllName))
            {
                throw new FileNotFoundException(dllName);
            }
            this.dll = new UnmanagedDll(dllName);
            this.ClrObjectToSexp = dll.GetFunction<ClrObjectToSexpDelegate>("clr_object_to_SEXP");
        }

        public ClrObjectToSexpDelegate ClrObjectToSexp { get; set; }

        private UnmanagedDll dll;


        public IntPtr GetFunctionAddress(string entryPointName)
        {
            return dll.GetFunctionAddress(entryPointName);
        }
    }

}


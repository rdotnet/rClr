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
            this.ClrObjectToSexp = Dll.GetFunction<ClrObjectToSexpDelegate>("clr_object_to_SEXP");
        }

        public ClrObjectToSexpDelegate ClrObjectToSexp { get; set; }

        public UnmanagedDll Dll
        {
            get
            {
                return dll;
            }
        }

        private UnmanagedDll dll;


        public IntPtr GetFunctionAddress(string entryPointName)
        {
            return Dll.GetFunctionAddress(entryPointName);
        }
    }

}


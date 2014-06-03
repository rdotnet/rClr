using System;
using RDotNet.NativeLibrary;
using System.Runtime.InteropServices;
using System.Reflection;
using System.IO;

namespace Rclr
{
    public class RclrUnmanagedDll : UnmanagedDll
    {
        public RclrUnmanagedDll(string dllName) : base(dllName)
        {
            if (!File.Exists(dllName))
            {
                throw new FileNotFoundException(dllName);
            }
            this.ClrObjectToSexp = GetFunction<UnmanagedRclrDll.ClrObjectToSexpDelegate>("clr_object_to_SEXP");
        }

        public UnmanagedRclrDll CreateWrapper()
        {
            return new UnmanagedRclrDll() { ClrObjectToSexp=this.ClrObjectToSexp };
        }

        public UnmanagedRclrDll.ClrObjectToSexpDelegate ClrObjectToSexp;
    }

}


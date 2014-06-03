using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace Rclr
{
    public class UnmanagedRclrDll
    {
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate IntPtr ClrObjectToSexpDelegate(IntPtr variant);
        public ClrObjectToSexpDelegate ClrObjectToSexp;
    }

    /// <summary>
    /// A helper class to inspect data and determind what it is converted to in the unmanaged code.
    /// </summary>
    /// <remarks>
    /// Acknowledgements go to Lim Bio Liong for some of this code. See http://limbioliong.wordpress.com/2011/09/04/using-variants-in-managed-code-part-1/ 
    /// and  http://limbioliong.wordpress.com/2011/03/20/c-interop-how-to-return-a-variant-from-an-unmanaged-function/. 
    /// Very useful and impressive series of articles.
    /// </remarks>
    public static class DataConversionHelper
    {
        // c:\Program Files\Windows Kits\8.0\Include\um\OAIdl.h
        //typedef /* [wire_marshal] */ struct tagVARIANT VARIANT;
        //struct tagVARIANT
        [StructLayout(LayoutKind.Sequential)]
        public struct Variant
        {
            public ushort vt;
            public ushort wReserved1;
            public ushort wReserved2;
            public ushort wReserved3;
            public Int32 data01;
            public Int32 data02;
        }
       
        public static UnmanagedRclrDll RclrNativeDll= null;
        static Int32 VariantClear(IntPtr pvarg)
        {
            if (RclrNativeDll == null)
                return VariantClearMs(pvarg);
            throw new NotSupportedException("Variant clear can only work with Windows");
        }

        static IntPtr ClrObjectToSexp(IntPtr variant)
        {
            if (RclrNativeDll == null)
                return ClrObjectToSexpMs(variant);
            return RclrNativeDll.ClrObjectToSexp(variant);
        }

        // TODO I will likely need some conditional compilation for Mono
        [DllImport(@"oleaut32.dll", SetLastError = true, CallingConvention = CallingConvention.StdCall)]
        private static extern Int32 VariantClearMs(IntPtr pvarg);

        const Int32 SizeOfNativeVariant = 16;


        [DllImport(@"rClrMs.dll", EntryPoint = "clr_object_to_SEXP", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr ClrObjectToSexpMs(IntPtr variant);

        /// <summary>
        /// Creates a pointer to a native SEXP. This method is for advanced operations, 
        /// where garbage collections are impacted. Gurus only.
        /// </summary>
        /// <param name="obj"></param>
        /// <returns></returns>
        public static IntPtr ClrObjectToSexp(object obj)
        {
            IntPtr pVariant = IntPtr.Zero;
            try
            {
                pVariant = CreateNativeVariantForObject(obj);
                return ClrObjectToSexp(pVariant);
            }
            catch
            {
                // We want to deallocate memory on error, but not on successful completion.
                // since the creation of a native variant for obj is what creates a handle in the CLR hosting
                // to prevent the garbage collection.
                FreeVariantMem(pVariant);
                throw;
            }
        }

        /// <summary>
        /// Gets a string helping to identify the COM variant to which an object is converted to an unmanaged data structure, if at all.
        /// </summary>
        /// <param name="obj">The object to check</param>
        /// <returns>A string such as "VT_ARRAY | VT_BOOL"</returns>
        public static string GetVariantTypename(object obj)
        {
            string vtString = "??";
            // GetNativeVariantForObject cannot deal with generic types.
            if (obj != null)
            {
                var t = obj.GetType();
                if (t.IsGenericType)
                    return string.Format("{0} cannot be converted to a native variant type as it is a generic type", t.FullName);
            }
            IntPtr pVariant = IntPtr.Zero;
            try
            {
                pVariant = CreateNativeVariantForObject(obj);
                vtString = GetVariantTypeString(pVariant);
            }
            finally
            {
                FreeVariantMem(pVariant);
            }
            return vtString;
        }

        private static string GetVariantTypeString( IntPtr pVariant)
        {
            VarEnum vt = GetVariantType(pVariant);
            return GetVariantTypeString(vt);
        }

        private static VarEnum GetVariantType(IntPtr pVariant)
        {
            Variant v = GetManagedVariant(pVariant);
            VarEnum vt = (VarEnum)(v.vt);
            return vt;
        }

        private static Variant GetManagedVariant(IntPtr pVariant)
        {
            Variant v = (Variant)Marshal.PtrToStructure(pVariant, typeof(Variant));
            return v;
        }

        private static void FreeVariantMem(IntPtr pVariant)
        {
            if (pVariant != IntPtr.Zero)
            {
                VariantClear(pVariant);
                Marshal.FreeCoTaskMem(pVariant);
            }
        }

        private static IntPtr CreateNativeVariantForObject(object obj)
        {
            IntPtr pVariant = Marshal.AllocHGlobal(SizeOfNativeVariant);
            Marshal.GetNativeVariantForObject(obj, pVariant);
            return pVariant;
        }

        /// <summary>
        /// Gets a string helping to identify the COM variant to which an object is converted to an unmanaged data structure, if at all.
        /// </summary>
        /// <param name="obj">An object or type name, from which to call a method</param>
        /// <param name="methodName">the name of a method to call on the object or class</param>
        /// <param name="arguments">The arguments to the method call</param>
        /// <returns>A string such as "VT_ARRAY | VT_BOOL" if the method call returns a bool[]</returns>
        public static string GetReturnedVariantTypename(object obj, string methodName, params object[] arguments)
        {
            if (obj == null)
                throw new ArgumentNullException("obj");
            else if (obj is Type)
                return GetVariantTypename(ClrFacade.InternalCallStaticMethod((Type)obj, methodName, false, arguments));
            else if (obj is string)
                return GetVariantTypename(ClrFacade.InternalCallStaticMethod(ClrFacade.GetType((string)obj), methodName, false, arguments));
            else
                return GetVariantTypename(ClrFacade.InternalCallInstanceMethod(obj, methodName, false, arguments));
        }

        private static string GetVariantTypeString(VarEnum vt)
        {
            VarEnum e;
            var vtNames = Enum.GetNames(typeof(VarEnum));
            foreach (var vtn in vtNames)
            {
                e = (VarEnum)Enum.Parse(typeof(VarEnum), vtn);
                if (e == vt)
                    return vtn;
                else if (vt == (VarEnum.VT_ARRAY | e))
                    return string.Concat("VT_ARRAY | ", vtn);
            }
            throw new NotSupportedException(string.Format("Could not find a valid VARIANT type for VarEnum code {0}", (int)vt));
        }
    }
}

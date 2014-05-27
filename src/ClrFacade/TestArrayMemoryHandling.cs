using System;
using System.Reflection;
using Rclr;

namespace Rclr
{
    /// <summary>
    /// Do not modify the .cs file: T4 generated class to support the unit tests for method binding
    /// </summary>
    /// <remarks>
    /// Added initially as a basis for unit test for issue https://r2clr.codeplex.com/workitem/70
    /// </remarks>
    public class TestArrayMemoryHandling
    {

        public object[] FieldArray_object;
        public static object[] CreateArray_object(int size) 
        {
            return (object[])Array.CreateInstance(typeof(object), size);
        }
        public static object[] CreateArray_object(int size, object value) 
        {
            var result =  (object[])Array.CreateInstance(typeof(object), size);
			for(int i = 0; i < result.Length; i++) result[i] = value;
			return result;
        }

        public string[] FieldArray_string;
        public static string[] CreateArray_string(int size) 
        {
            return (string[])Array.CreateInstance(typeof(string), size);
        }
        public static string[] CreateArray_string(int size, string value) 
        {
            var result =  (string[])Array.CreateInstance(typeof(string), size);
			for(int i = 0; i < result.Length; i++) result[i] = value;
			return result;
        }

        public double[] FieldArray_double;
        public static double[] CreateArray_double(int size) 
        {
            return (double[])Array.CreateInstance(typeof(double), size);
        }
        public static double[] CreateArray_double(int size, double value) 
        {
            var result =  (double[])Array.CreateInstance(typeof(double), size);
			for(int i = 0; i < result.Length; i++) result[i] = value;
			return result;
        }

        public float[] FieldArray_float;
        public static float[] CreateArray_float(int size) 
        {
            return (float[])Array.CreateInstance(typeof(float), size);
        }
        public static float[] CreateArray_float(int size, float value) 
        {
            var result =  (float[])Array.CreateInstance(typeof(float), size);
			for(int i = 0; i < result.Length; i++) result[i] = value;
			return result;
        }

        public int[] FieldArray_int;
        public static int[] CreateArray_int(int size) 
        {
            return (int[])Array.CreateInstance(typeof(int), size);
        }
        public static int[] CreateArray_int(int size, int value) 
        {
            var result =  (int[])Array.CreateInstance(typeof(int), size);
			for(int i = 0; i < result.Length; i++) result[i] = value;
			return result;
        }

        public long[] FieldArray_long;
        public static long[] CreateArray_long(int size) 
        {
            return (long[])Array.CreateInstance(typeof(long), size);
        }
        public static long[] CreateArray_long(int size, long value) 
        {
            var result =  (long[])Array.CreateInstance(typeof(long), size);
			for(int i = 0; i < result.Length; i++) result[i] = value;
			return result;
        }

        public bool[] FieldArray_bool;
        public static bool[] CreateArray_bool(int size) 
        {
            return (bool[])Array.CreateInstance(typeof(bool), size);
        }
        public static bool[] CreateArray_bool(int size, bool value) 
        {
            var result =  (bool[])Array.CreateInstance(typeof(bool), size);
			for(int i = 0; i < result.Length; i++) result[i] = value;
			return result;
        }

        public DateTime[] FieldArray_DateTime;
        public static DateTime[] CreateArray_DateTime(int size) 
        {
            return (DateTime[])Array.CreateInstance(typeof(DateTime), size);
        }
        public static DateTime[] CreateArray_DateTime(int size, DateTime value) 
        {
            var result =  (DateTime[])Array.CreateInstance(typeof(DateTime), size);
			for(int i = 0; i < result.Length; i++) result[i] = value;
			return result;
        }

        public TimeSpan[] FieldArray_TimeSpan;
        public static TimeSpan[] CreateArray_TimeSpan(int size) 
        {
            return (TimeSpan[])Array.CreateInstance(typeof(TimeSpan), size);
        }
        public static TimeSpan[] CreateArray_TimeSpan(int size, TimeSpan value) 
        {
            var result =  (TimeSpan[])Array.CreateInstance(typeof(TimeSpan), size);
			for(int i = 0; i < result.Length; i++) result[i] = value;
			return result;
        }

        public byte[] FieldArray_byte;
        public static byte[] CreateArray_byte(int size) 
        {
            return (byte[])Array.CreateInstance(typeof(byte), size);
        }
        public static byte[] CreateArray_byte(int size, byte value) 
        {
            var result =  (byte[])Array.CreateInstance(typeof(byte), size);
			for(int i = 0; i < result.Length; i++) result[i] = value;
			return result;
        }

        public char[] FieldArray_char;
        public static char[] CreateArray_char(int size) 
        {
            return (char[])Array.CreateInstance(typeof(char), size);
        }
        public static char[] CreateArray_char(int size, char value) 
        {
            var result =  (char[])Array.CreateInstance(typeof(char), size);
			for(int i = 0; i < result.Length; i++) result[i] = value;
			return result;
        }

        public Type[] FieldArray_Type;
        public static Type[] CreateArray_Type(int size) 
        {
            return (Type[])Array.CreateInstance(typeof(Type), size);
        }
        public static Type[] CreateArray_Type(int size, Type value) 
        {
            var result =  (Type[])Array.CreateInstance(typeof(Type), size);
			for(int i = 0; i < result.Length; i++) result[i] = value;
			return result;
        }
		// To test the type of empty vectors:
        public static bool CheckElementType(Array array, Type expectedElementType)
        {
            return (array.GetType().GetElementType() == expectedElementType);
        }
    }
}

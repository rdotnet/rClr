using System;
using System.Reflection;
using Rclr;

namespace Rclr
{
    /// <summary>
    /// Do not modify the .cs file: T4 generated class to support the unit tests for method binding
    /// </summary>
    /// <remarks>
    /// Added as a basis for unit test for issue https://r2clr.codeplex.com/workitem/70
    /// </remarks>
    public class TestArrayMemoryHandling
    {
        public object[] FieldArray_object;
        public object[] CreateArray_object(int size) 
        {
            return (object[])Array.CreateInstance(typeof(object), size);
        }
        public string[] FieldArray_string;
        public string[] CreateArray_string(int size) 
        {
            return (string[])Array.CreateInstance(typeof(string), size);
        }
        public double[] FieldArray_double;
        public double[] CreateArray_double(int size) 
        {
            return (double[])Array.CreateInstance(typeof(double), size);
        }
        public float[] FieldArray_float;
        public float[] CreateArray_float(int size) 
        {
            return (float[])Array.CreateInstance(typeof(float), size);
        }
        public int[] FieldArray_int;
        public int[] CreateArray_int(int size) 
        {
            return (int[])Array.CreateInstance(typeof(int), size);
        }
        public bool[] FieldArray_bool;
        public bool[] CreateArray_bool(int size) 
        {
            return (bool[])Array.CreateInstance(typeof(bool), size);
        }
        public DateTime[] FieldArray_DateTime;
        public DateTime[] CreateArray_DateTime(int size) 
        {
            return (DateTime[])Array.CreateInstance(typeof(DateTime), size);
        }
        public byte[] FieldArray_byte;
        public byte[] CreateArray_byte(int size) 
        {
            return (byte[])Array.CreateInstance(typeof(byte), size);
        }
        public char[] FieldArray_char;
        public char[] CreateArray_char(int size) 
        {
            return (char[])Array.CreateInstance(typeof(char), size);
        }
    }
}

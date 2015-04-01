using RDotNet.Utilities;

namespace Rclr
{
    /// <summary>
    /// Extension methods to facilitate data marshalling between R and the CLR
    /// </summary>
    public static class DataConverterExtensions
    {
        public static double[][] ToDouble(this float[][] array)
        {
            return ArrayConverter.ArrayConvertAll<float, double>(array, x => (double)x);
        }

        public static double[,] ToDoubleRect(this float[][] array)
        {
            return array.ToDouble().ToRect();
        }

        public static bool IsRectangular<T>(this T[][] array)
        {
            if (array.Length == 0)
                return true;
            else
            {
                if (array[0] == null)
                    return false;
                int firstLen = array[0].Length;
                for (int i = 1; i < array.Length; i++)
                {
                    if (array[i] == null) return false;
                    else if (array[i].Length != firstLen) return false;
                }
            }
            return true;
        }

        public static T[,] ToRect<T>(this T[][] array)
        {
            var result = new T[array.Length, array[0].Length];
            for (int i = 0; i < array.Length; i++)
                for (int j = 0; j < array[0].Length; j++)
                    result[i, j] = array[i][j];
            return result;
        }

        public static double[,] ToDoubleRect(this float[,] array)
        {
            return ArrayConverter.ArrayConvertAll<float, double>(array, x => (double)x);
        }

        public static float[,] ToFloatRect(this float[][] array)
        {
            var result = new float[array.Length, array[0].Length];
            for (int i = 0; i < array.Length; i++)
                for (int j = 0; j < array[0].Length; j++)
                    result[i, j] = array[i][j];
            return result;
        }
    }
}
namespace Rclr
{
    /// <summary>
    /// Extension methods to facilitate data marshalling between R and the CLR
    /// </summary>
    public static class DataConverterExtensions
    {
        public static double[][] ToDouble(this float[][] array)
        {
            var result = new double[array.Length][];
            for (int i = 0; i < array.Length; i++)
            {
                result[i] = new double[array[i].Length];
                for (int j = 0; j < array[i].Length; j++)
                    result[i][j] = array[i][j];
            }
            return result;
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
            int n = array.GetLength(0);
            int m = array.GetLength(1);
            var result = new double[n, m];
            for (int i = 0; i < n; i++)
                for (int j = 0; j < m; j++)
                    result[i, j] = array[i, j];
            return result;
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
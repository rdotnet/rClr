using System;
using System.Collections.Generic;
using System.Text;

namespace Rclr
{
    /// <summary>
    /// Interface for objects that are convering CLR objects to a representation in R
    /// </summary>
    /// <remarks>Currently the only concrete implementation is a data converter that uses RDotNet 
    /// to expose CLR objects to R</remarks>
    public interface IDataConverter
    {
        object ConvertToR(object obj);
        object ConvertFromR(IntPtr pointer, int sexptype);

        // TODO: this should not be here, but for now a convenient way to access Rf_error via R.NET
        void Error(string msg);
    }
}

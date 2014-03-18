using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;

namespace Rclr
{
    /// <summary>
    /// 
    /// </summary>
    /// <remarks>
    /// https://rclr.codeplex.com/workitem/15
    /// </remarks>
    internal class RclrBinder : Binder
    {
        private Binder defaultBinder = System.Type.DefaultBinder;

        public override FieldInfo BindToField(BindingFlags bindingAttr, FieldInfo[] match, object value, System.Globalization.CultureInfo culture)
        {
            return defaultBinder.BindToField(bindingAttr, match, value, culture);
        }

        public override MethodBase BindToMethod(BindingFlags bindingAttr, MethodBase[] match, ref object[] args, ParameterModifier[] modifiers, System.Globalization.CultureInfo culture, string[] names, out object state)
        {
            return defaultBinder.BindToMethod(bindingAttr, match, ref args, modifiers, culture, names, out state);
        }

        public override object ChangeType(object value, Type type, System.Globalization.CultureInfo culture)
        {
            return defaultBinder.ChangeType(value, type, culture);
        }

        public override void ReorderArgumentArray(ref object[] args, object state)
        {
            defaultBinder.ReorderArgumentArray(ref args, state);
        }

        public override MethodBase SelectMethod(BindingFlags bindingAttr, MethodBase[] match, Type[] types, ParameterModifier[] modifiers)
        {
            var defaultMatch = defaultBinder.SelectMethod(bindingAttr, match, types, modifiers);
            if (defaultMatch != null)
                return defaultMatch;
            // otherwise, let's have a parameter matching that accepts double[] to float[] conversions. 
            // KLUDGE Note that this is likely to not be stringent enough and missing some things.
            // Unfortunately there is is no easy way in the Binder API to inject a specific type conversion, 
            // which would be very convenient here.
            List<MethodBase> results = new List<MethodBase>();
            for (int i = 0; i < match.Length; i++)
            {
                ParameterInfo[] pInfos = match[i].GetParameters();
                if (pInfos.Length != types.Length)
                    continue;
                bool accepted = true;
                for (int j = 0; j < types.Length; j++)
                {
                    Type paramType = pInfos[j].ParameterType;
                    if (paramType == types[j])
                        continue;
                    if (paramType == typeof(Object))
                        continue;
                    if (paramType == typeof(float[]) && types[j] == typeof(double[]))
                        continue;
                    else
                    {
                        if (!paramType.IsAssignableFrom(types[j]))
                        {
                            accepted = false;
                            break;
                        }
                    }
                }
                if (accepted)
                    results.Add(match[i]);
            }
            if (results.Count > 0)
                return results[0];
            else
                return null;
        }

        public override PropertyInfo SelectProperty(BindingFlags bindingAttr, PropertyInfo[] match, Type returnType, Type[] indexes, ParameterModifier[] modifiers)
        {
            return defaultBinder.SelectProperty(bindingAttr, match, returnType, indexes, modifiers);
        }
    }
}

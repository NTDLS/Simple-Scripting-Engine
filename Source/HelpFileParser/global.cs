using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Resources;
using Microsoft.Win32;

namespace HelpFileParser
{
    static public class global
    {
        public const string RegistryKeyString = "SOFTWARE\\NetWorkDLS\\Simple Scripting Engine";

        static public string GetRegistryString(string subPath, string valueName, string Default)
        {
            ResourceManager rm = new ResourceManager(typeof(string));
            RegistryKey regKey = Registry.LocalMachine;
            RegistryKey regSubKey = regKey.OpenSubKey(RegistryKeyString + "\\" + subPath);

            object value = regSubKey.GetValue(valueName);

            if (value != null)
            {
                string stringValue = value.ToString();

                if (stringValue == null)
                {
                    return Default;
                }

                return stringValue;
            }

            return null;
        }

        static public string GetRegistryString(string subPath, string valueName)
        {
            return GetRegistryString(subPath, valueName, "");
        }
    }
}

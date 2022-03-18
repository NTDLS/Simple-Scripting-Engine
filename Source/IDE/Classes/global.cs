using System;
using Microsoft.Win32;
using System.Resources;
using System.Reflection;

namespace SSIDE.Classes
{
    public static partial class global
    {
        public const string RegistryKey = "SOFTWARE\\NetworkDLS\\Simple Scripting Engine";
        public const string CodeFileExtension = ".ss";
        public const string ProjectFileExtension = ".ssepx";

        static public void SetRegistryString(string subPath, string valueName, string value)
        {
            ResourceManager rm = new ResourceManager(typeof(string));
            RegistryKey regKey = Registry.LocalMachine;
            RegistryKey regSubKey = regKey.OpenSubKey(RegistryKey + "\\" + subPath, true);
            regSubKey.SetValue(valueName, value);
        }
        static public string GetRegistryString(string subPath, string valueName)
        {
            return GetRegistryString(subPath, valueName, "");
        }

        static public string GetRegistryString(string subPath, string valueName, string Default)
        {
            ResourceManager rm = new ResourceManager(typeof(string));
            RegistryKey regKey = Registry.LocalMachine;
            RegistryKey regSubKey = regKey.OpenSubKey(RegistryKey + "\\" + subPath);

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

        static public void SetRegistryInt(string subPath, string valueName, int value)
        {
            ResourceManager rm = new ResourceManager(typeof(string));
            RegistryKey regKey = Registry.LocalMachine;
            RegistryKey regSubKey = regKey.OpenSubKey(RegistryKey + "\\" + subPath, true);
            regSubKey.SetValue(valueName, value);
        }

        static public int GetRegistryInt(string subPath, string valueName)
        {
            return GetRegistryInt(subPath, valueName, 0);
        }

        static public int GetRegistryInt(string subPath, string valueName, int Default)
        {
            ResourceManager rm = new ResourceManager(typeof(string));
            RegistryKey regKey = Registry.LocalMachine;
            RegistryKey regSubKey = regKey.OpenSubKey(RegistryKey + "\\" + subPath);

            Object value = regSubKey.GetValue(valueName);

            if (value == null)
            {
                return Default;
            }

            return int.Parse(value.ToString());
        }

        static public void SetRegistryBool(string subPath, string valueName, bool value)
        {
            SetRegistryInt(subPath, valueName, Convert.ToInt32(value));
        }

        static public bool GetRegistryBool(string subPath, string valueName)
        {
            return GetRegistryInt(subPath, valueName) != 0;
        }

        static public bool GetRegistryBool(string subPath, string valueName, bool Default)
        {
            return GetRegistryInt(subPath, valueName, Convert.ToInt32(Default)) != 0;
        }

    }
}

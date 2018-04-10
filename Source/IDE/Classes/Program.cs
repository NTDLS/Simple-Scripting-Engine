using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using SSIDE.Forms;
using System.IO;
using SSIDE.Classes;

namespace SSIDE
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string []Args)
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            frmMain mainForm = new frmMain();

            foreach (string fileName in Args)
            {
                if (Path.GetExtension(fileName).ToLower() == global.ProjectFileExtension)
                {
                    mainForm.LoadProjectFile(fileName);
                }
                else
                {
                    mainForm.AddNewTab(fileName);
                }
            }

            mainForm.ShowDialog();
        }
    }
}

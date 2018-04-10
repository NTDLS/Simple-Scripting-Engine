using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SSIDE.Classes;
using Newtonsoft.Json;

namespace HelpFileParser
{
    class Program
    {
        private static List<string> types = new List<string>
        {
            "String", "Numeric", "SQL.Connection", "SQL.RecordSet", "File", "List", "Generic", "Socket.Server", "Socket.Client", "Exception", "XML.Writer", "XML.Reader"
        };

        private static HtmlHelpViewer.Viewer _HelpContext = null;
        static private HtmlHelpViewer.Viewer HelpContext
        {
            get
            {
                if (_HelpContext == null)
                {
                    string sDocumentation = "..\\..\\..\\..\\Documentation\\Help.chm";
                    _HelpContext = new HtmlHelpViewer.Viewer();
                    if (!_HelpContext.OpenFile(sDocumentation))
                    {
                        _HelpContext.Close();
                        _HelpContext = null;
                    }
                }

                return _HelpContext;
            }
        }

        static string GetDocumentationText(string keyWord)
        {
            try
            {
                HtmlHelp.IndexTopic indexTopic = HelpContext.GetTopic("[" + keyWord + "]");

                if (indexTopic != null)
                {
                    string fileContents = indexTopic.FileContents;

                    int startPos = indexTopic.FileContents.IndexOf("<!-- ##BEGIN_DESCRIPTION## -->");
                    if (startPos > 0)
                    {
                        startPos += 31;
                        int endPos = indexTopic.FileContents.IndexOf("<!-- ##END_DESCRIPTION## -->", startPos);
                        if (endPos > 0)
                        {
                            return fileContents.Substring(startPos, endPos - startPos);
                        }
                    }
                }
            }
            catch
            {
            }

            return null;
        }

        [STAThread]
        static void Main(string[] args)
        {
            AutoCompleteItems items = new AutoCompleteItems();

            HashSet<string> completed = new HashSet<string>();

            int KeywordLinksCount = HelpContext.Reader.Index.Count(HtmlHelp.IndexType.KeywordLinks);
            for (int i = 0; i < KeywordLinksCount; i++)
            {
                HtmlHelp.IndexItem indexItem = (HtmlHelp.IndexItem)HelpContext.Reader.Index.KLinks[i];

                foreach (HtmlHelp.IndexTopic topic in indexItem.Topics)
                {
                    string title = topic.Title.Replace(" :: ", ".").Trim();

                    if (completed.Contains(title) == false)
                    {
                        string fileContents = topic.FileContents;

                        int startPos = fileContents.IndexOf("<!-- ##BEGIN_DESCRIPTION## -->");
                        if (startPos > 0)
                        {
                            startPos += 31;
                            int endPos = fileContents.IndexOf("<!-- ##END_DESCRIPTION## -->", startPos);
                            if (endPos > 0)
                            {
                                string typelessName = title;

                                foreach (string typeName in types)
                                {
                                    typelessName = typelessName.Replace(typeName + ".", "");
                                }

                                completed.Add(title);

                                items.Add(new AutoCompleteItem
                                {
                                    Topic = title,
                                    Keyword = title.ToLower(),
                                    URL = topic.Local.Trim(),
                                    Content = fileContents.Substring(startPos, endPos - startPos).Trim(),
                                });

                                if (typelessName != title)
                                {
                                    completed.Add(typelessName);

                                    items.Add(new AutoCompleteItem
                                    {
                                        Type = title.Replace(typelessName, "").Trim('.'),
                                        Topic = typelessName,
                                        Keyword = typelessName.ToLower(),
                                        URL = topic.Local.Trim(),
                                        Content = fileContents.Substring(startPos, endPos - startPos).Trim(),
                                    });
                                }
                            }
                        }
                    }
                }
            }

            string json = JsonConvert.SerializeObject(items);

            System.IO.File.WriteAllText("..\\..\\..\\..\\Documentation\\AutoComplete.json", json);
        }
    }
}

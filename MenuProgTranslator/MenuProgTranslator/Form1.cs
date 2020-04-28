using System;
using System.Text;
using System.IO;
using System.Windows.Forms;
using Excel = Microsoft.Office.Interop.Excel;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;

namespace MenuProgTranslator
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            string curFolder = Path.GetDirectoryName(Application.ExecutablePath);
            string progMenuPath = Path.GetFullPath (curFolder +"\\..\\bin\\varie\\prog");
            string xlsPath = curFolder + "\\translations.xlsx";
            translate(progMenuPath, xlsPath);
            //translate(@"C:\rhea\rheaSRC\gpu-fts-nestle-2019\bin\varie\prog", @"C:\rhea\rheaSRC\gpu-fts-nestle-2019\MenuProgTranslator\translations.xlsx");
        }

        private void log(string s)
        {
            txtLogger.AppendText(logSpacer +s + "\r\n");
            txtLogger.ScrollToCaret();
        }

        private int logTab = 0;
        private string logSpacer = "";

        private void logIncTab()
        {
            logSpacer = "";
            logTab++;
            for (int i=0; i<logTab; i++)
                logSpacer+= " ";
        }

        private void logDecTab()
        {
            logSpacer = "";
            if (logTab > 0)
            {
                logTab--;
                for (int i = 0; i < logTab; i++)
                    logSpacer += " ";
            }
        }


        private string getValue(Excel.Worksheet sheet, int row, int cell)
        {
            string s = "";
            try
            {
                s = sheet.Cells[row, cell].Value.ToString().Trim();
            }
            catch { }

            return s;
        }

        private int getLastGoodRow(Excel.Worksheet sheet)
        {
            int riga = 1;
            int n = 0;
            while (true)
            {
                if (getValue(sheet, riga, 1) == "")
                    n++;
                else
                    n = 0;
                riga++;

                if (n == 3)
                    return (riga - 4);
            }
        }

        private string toUTF8(string s)
        {
            byte[] utf8Sequence = Encoding.Convert(Encoding.Default, Encoding.UTF8, Encoding.Default.GetBytes(s));
            return Encoding.UTF8.GetString(utf8Sequence);
        }

        private void translateSpecificLang(String fullPathToMainMenuProgFolder, Excel.Sheets sheets, int[] lastGoodRow, int langColumn, string langISO)
        {
            log("processing language " + langISO);
            logIncTab();

            string fileIN, fileOUT;

            fileIN = "index_template.html";
            fileOUT = "index_" + langISO + ".html";
            translateSpecificFile(fullPathToMainMenuProgFolder, fileIN, fileOUT, sheets, lastGoodRow, langColumn, langISO);

            fileIN = "Task.js";
            fileOUT = "Task_" + langISO + ".js";
            translateSpecificFile(fullPathToMainMenuProgFolder+"/js", fileIN, fileOUT, sheets, lastGoodRow, langColumn, langISO);

            logDecTab();
        }

        private void translateSpecificFile(String folderPath, String inputFileName, String outputFileName, Excel.Sheets sheets, int[] lastGoodRow, int langColumn, string langISO)
        {
            log("reading " + folderPath + "/" + inputFileName);
            string template = File.ReadAllText(folderPath + "/" + inputFileName, Encoding.UTF8);
            

            //bandiera della lingua corrente
            string label = "$_CUR_LANG_ISO_2_LETTERS";
            while (true)
            {
                int iFound = template.IndexOf(label);
                if (iFound < 0)
                    break;
                template = template.Substring(0, iFound) + langISO + template.Substring(iFound + label.Length);
            }



            for (int iSheet = 1; iSheet <= sheets.Count; iSheet++)
            {
                Excel.Worksheet sheet = (Excel.Worksheet)sheets[iSheet];

                for (int row = 2; row <= lastGoodRow[iSheet]; row++)
                {
                    label = getValue(sheet, row, 1);
                    if (label == "")
                        continue;
                    string translation = getValue(sheet, row, langColumn);
                    if (translation == "")
                        translation = getValue(sheet, row, 2); //default to GB

                    label = toUTF8(label);
                    int lenOfLabel = label.Length;
                    translation = toUTF8(translation);
                    translation = translation.Replace("\"", "&quot;");

                    int startIndex = 0;
                    while (true)
                    {
                        int iFound = template.IndexOf(label, startIndex, StringComparison.InvariantCultureIgnoreCase);
                        if (iFound < 0)
                            break;

                        //per assicurarmi che sia una "whole word", verifico che il carattere che segue non sia una lettera o un _
                        string c = template.Substring(iFound + lenOfLabel, 1);
                        if (char.IsLetterOrDigit(c[0]) || c == "_")
                        {
                            startIndex = iFound + lenOfLabel;
                        }
                        else
                        {
                            template = template.Substring(0, iFound) + translation + template.Substring(iFound + lenOfLabel);
                            startIndex = iFound;
                        }
                    }

                    Application.DoEvents();
                }
            }

            log("writing " +folderPath + "/" + outputFileName);
            System.Text.Encoding encoder = new System.Text.UTF8Encoding(false);
            File.WriteAllText (folderPath + "/" + outputFileName, template, encoder);
        }

        private void translate(String fullPathToMainMenuProgFolder, String fullPathToExcelFileWithTranslation)
        {
            log("STARTING");
            logIncTab();
                log("xls = " + fullPathToExcelFileWithTranslation);
                log("prog folder = " + fullPathToMainMenuProgFolder);
            logDecTab();

            bool bCanDoJob = true;
            Excel.Application xlApp = new Excel.Application();
            Excel.Workbook wb = xlApp.Workbooks.Open(fullPathToExcelFileWithTranslation);
            Excel.Sheets sheets = xlApp.Worksheets;

            //recupera un elenco di lingue (dal foglio 1)
            int numLanguages = 0;
            Excel.Worksheet sheet1 = (Excel.Worksheet)sheets[1];
            for (int i=2; i<100; i++)
            {
                string langISO = getValue(sheet1, 1, i);
                if (langISO == "")
                {
                    numLanguages = i - 2;
                    break;
                }
            }

            //conta il numero di righe buone per ogni foglio xls e si assicura che ci siano tutte le colonne con le lingue
            int[] lastGoodRow = new int[1 + sheets.Count];
            for (int i = 1; i <= sheets.Count; i++)
            {
                Excel.Worksheet sheet = (Excel.Worksheet)sheets[i];
                //conta il numero di righe buone
                lastGoodRow[i] = getLastGoodRow(sheet);

                for (int t = 0; t < numLanguages; t++)
                {
                    string langISO = getValue(sheet1, 1, t+2);
                    if (getValue(sheet, 1, t + 2) != langISO)
                    {
                        MessageBox.Show("ATTENZIONE: il foglio di lavoro numero " + (t + 2) + " non contiene la colonna con la lingua " + langISO);
                        bCanDoJob = false;
                        break;
                    }
                }
            }

            if (bCanDoJob)
            {
                for (int iLang = 0; iLang < numLanguages; iLang++)
                {
                    int langCol = iLang + 2;
                    string langISO = getValue(sheet1, 1, langCol);
                    Application.DoEvents();
                    translateSpecificLang (fullPathToMainMenuProgFolder, sheets, lastGoodRow, langCol, langISO);
                }
                log("Finished");
            }

            wb.Close();
            xlApp.Quit();
            Marshal.ReleaseComObject(wb);
            Marshal.ReleaseComObject(xlApp);
            Application.Exit();
        }
    }
}

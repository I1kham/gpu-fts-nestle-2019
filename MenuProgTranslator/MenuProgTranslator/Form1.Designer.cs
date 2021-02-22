namespace MenuProgTranslator
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.button1 = new System.Windows.Forms.Button();
            this.txtLogger = new System.Windows.Forms.TextBox();
            this.btnChangePathOfTranslationFile = new System.Windows.Forms.Button();
            this.labPathOfTranslationFile = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.btnChangePathOfMenuProgFolder = new System.Windows.Forms.Button();
            this.labPathOfMenuProgFolder = new System.Windows.Forms.Label();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.button2 = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // button1
            // 
            this.button1.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(255)))), ((int)(((byte)(192)))));
            this.button1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.button1.Location = new System.Drawing.Point(14, 93);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(271, 41);
            this.button1.TabIndex = 0;
            this.button1.Text = "START";
            this.button1.UseVisualStyleBackColor = false;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // txtLogger
            // 
            this.txtLogger.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.txtLogger.Location = new System.Drawing.Point(12, 140);
            this.txtLogger.Multiline = true;
            this.txtLogger.Name = "txtLogger";
            this.txtLogger.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.txtLogger.Size = new System.Drawing.Size(890, 245);
            this.txtLogger.TabIndex = 1;
            // 
            // btnChangePathOfTranslationFile
            // 
            this.btnChangePathOfTranslationFile.Location = new System.Drawing.Point(204, 8);
            this.btnChangePathOfTranslationFile.Name = "btnChangePathOfTranslationFile";
            this.btnChangePathOfTranslationFile.Size = new System.Drawing.Size(79, 23);
            this.btnChangePathOfTranslationFile.TabIndex = 2;
            this.btnChangePathOfTranslationFile.Text = "change...";
            this.btnChangePathOfTranslationFile.UseVisualStyleBackColor = true;
            this.btnChangePathOfTranslationFile.Click += new System.EventHandler(this.btnChangePathOfTranslationFile_Click);
            // 
            // labPathOfTranslationFile
            // 
            this.labPathOfTranslationFile.BackColor = System.Drawing.Color.YellowGreen;
            this.labPathOfTranslationFile.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labPathOfTranslationFile.Location = new System.Drawing.Point(289, 9);
            this.labPathOfTranslationFile.Name = "labPathOfTranslationFile";
            this.labPathOfTranslationFile.Size = new System.Drawing.Size(613, 23);
            this.labPathOfTranslationFile.TabIndex = 3;
            this.labPathOfTranslationFile.Text = "none";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(11, 11);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(187, 16);
            this.label1.TabIndex = 4;
            this.label1.Text = "Path of translation.xlsx file";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.Location = new System.Drawing.Point(25, 47);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(173, 16);
            this.label2.TabIndex = 5;
            this.label2.Text = "Path of menuprog folder";
            // 
            // btnChangePathOfMenuProgFolder
            // 
            this.btnChangePathOfMenuProgFolder.Location = new System.Drawing.Point(204, 44);
            this.btnChangePathOfMenuProgFolder.Name = "btnChangePathOfMenuProgFolder";
            this.btnChangePathOfMenuProgFolder.Size = new System.Drawing.Size(79, 23);
            this.btnChangePathOfMenuProgFolder.TabIndex = 6;
            this.btnChangePathOfMenuProgFolder.Text = "change...";
            this.btnChangePathOfMenuProgFolder.UseVisualStyleBackColor = true;
            this.btnChangePathOfMenuProgFolder.Click += new System.EventHandler(this.btnChangePathOfMenuProgFolder_Click);
            // 
            // labPathOfMenuProgFolder
            // 
            this.labPathOfMenuProgFolder.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(255)))), ((int)(((byte)(255)))));
            this.labPathOfMenuProgFolder.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labPathOfMenuProgFolder.Location = new System.Drawing.Point(289, 44);
            this.labPathOfMenuProgFolder.Name = "labPathOfMenuProgFolder";
            this.labPathOfMenuProgFolder.Size = new System.Drawing.Size(613, 23);
            this.labPathOfMenuProgFolder.TabIndex = 7;
            this.labPathOfMenuProgFolder.Text = "none";
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.FileName = "openFileDialog1";
            // 
            // button2
            // 
            this.button2.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(255)))), ((int)(((byte)(192)))));
            this.button2.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.button2.Location = new System.Drawing.Point(631, 93);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(271, 41);
            this.button2.TabIndex = 8;
            this.button2.Text = "DEBUG: ONLY GB";
            this.button2.UseVisualStyleBackColor = false;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(914, 410);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.labPathOfMenuProgFolder);
            this.Controls.Add(this.btnChangePathOfMenuProgFolder);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.labPathOfTranslationFile);
            this.Controls.Add(this.btnChangePathOfTranslationFile);
            this.Controls.Add(this.txtLogger);
            this.Controls.Add(this.button1);
            this.Name = "Form1";
            this.Text = "Menu Prog Translator";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.TextBox txtLogger;
        private System.Windows.Forms.Button btnChangePathOfTranslationFile;
        private System.Windows.Forms.Label labPathOfTranslationFile;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button btnChangePathOfMenuProgFolder;
        private System.Windows.Forms.Label labPathOfMenuProgFolder;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.Button button2;
    }
}


using Lunar;
using System;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Windows.Forms;
using System.Xml;

namespace X3MP_Launcher
{
    partial class X3MPLauncher
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
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
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(X3MPLauncher));
            this.connectButton = new System.Windows.Forms.Button();
            this.IpField = new System.Windows.Forms.TextBox();
            this.NameInput = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.PortInput = new System.Windows.Forms.NumericUpDown();
            this.label4 = new System.Windows.Forms.Label();
            this.closeButton = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.PortInput)).BeginInit();
            this.SuspendLayout();
            // 
            // connectButton
            // 
            this.connectButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(24)))), ((int)(((byte)(30)))), ((int)(((byte)(54)))));
            resources.ApplyResources(this.connectButton, "connectButton");
            this.connectButton.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(158)))), ((int)(((byte)(161)))), ((int)(((byte)(176)))));
            this.connectButton.Name = "connectButton";
            this.connectButton.UseVisualStyleBackColor = false;
            this.connectButton.Click += new System.EventHandler(this.connect_Click);
            // 
            // IpField
            // 
            this.IpField.AccessibleRole = System.Windows.Forms.AccessibleRole.IpAddress;
            this.IpField.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(46)))), ((int)(((byte)(51)))), ((int)(((byte)(73)))));
            this.IpField.BorderStyle = System.Windows.Forms.BorderStyle.None;
            resources.ApplyResources(this.IpField, "IpField");
            this.IpField.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(158)))), ((int)(((byte)(161)))), ((int)(((byte)(176)))));
            this.IpField.Name = "IpField";
            // 
            // NameInput
            // 
            this.NameInput.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(46)))), ((int)(((byte)(51)))), ((int)(((byte)(73)))));
            this.NameInput.BorderStyle = System.Windows.Forms.BorderStyle.None;
            resources.ApplyResources(this.NameInput, "NameInput");
            this.NameInput.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(158)))), ((int)(((byte)(161)))), ((int)(((byte)(176)))));
            this.NameInput.Name = "NameInput";
            // 
            // label1
            // 
            resources.ApplyResources(this.label1, "label1");
            this.label1.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(158)))), ((int)(((byte)(161)))), ((int)(((byte)(176)))));
            this.label1.Name = "label1";
            // 
            // label2
            // 
            resources.ApplyResources(this.label2, "label2");
            this.label2.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(158)))), ((int)(((byte)(161)))), ((int)(((byte)(176)))));
            this.label2.Name = "label2";
            // 
            // label3
            // 
            resources.ApplyResources(this.label3, "label3");
            this.label3.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(158)))), ((int)(((byte)(161)))), ((int)(((byte)(176)))));
            this.label3.Name = "label3";
            // 
            // PortInput
            // 
            this.PortInput.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(46)))), ((int)(((byte)(51)))), ((int)(((byte)(73)))));
            this.PortInput.BorderStyle = System.Windows.Forms.BorderStyle.None;
            resources.ApplyResources(this.PortInput, "PortInput");
            this.PortInput.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(158)))), ((int)(((byte)(161)))), ((int)(((byte)(176)))));
            this.PortInput.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.PortInput.Name = "PortInput";
            this.PortInput.Value = new decimal(new int[] {
            13337,
            0,
            0,
            0});
            // 
            // label4
            // 
            resources.ApplyResources(this.label4, "label4");
            this.label4.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(158)))), ((int)(((byte)(161)))), ((int)(((byte)(176)))));
            this.label4.Name = "label4";
            // 
            // closeButton
            // 
            resources.ApplyResources(this.closeButton, "closeButton");
            this.closeButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(24)))), ((int)(((byte)(30)))), ((int)(((byte)(54)))));
            this.closeButton.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(158)))), ((int)(((byte)(161)))), ((int)(((byte)(176)))));
            this.closeButton.Name = "closeButton";
            this.closeButton.UseVisualStyleBackColor = false;
            this.closeButton.Click += new System.EventHandler(this.closeButton_Click);
            // 
            // X3MPLauncher
            // 
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(24)))), ((int)(((byte)(30)))), ((int)(((byte)(54)))));
            this.Controls.Add(this.closeButton);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.PortInput);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.NameInput);
            this.Controls.Add(this.connectButton);
            this.Controls.Add(this.IpField);
            this.Controls.Add(this.label3);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "X3MPLauncher";
            this.TopMost = true;
            ((System.ComponentModel.ISupportInitialize)(this.PortInput)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        private void connect_Click(object sender, EventArgs e)
        {
            XmlTextWriter configWriter = new XmlTextWriter("x3mp.xml", null);
            configWriter.Indentation = 4;
            configWriter.Formatting = Formatting.Indented;

            configWriter.WriteStartDocument();
            configWriter.WriteStartElement("config");
            configWriter.WriteStartElement("server");

            configWriter.WriteElementString("username", this.NameInput.Text);

            configWriter.WriteElementString("ip", this.IpField.Text);

            configWriter.WriteElementString("port", this.PortInput.Value.ToString());

            configWriter.WriteEndElement();
            configWriter.WriteElementString("local", "0");
            configWriter.WriteElementString("debug", "0");
            configWriter.WriteEndElement();
            configWriter.WriteEndDocument();
            configWriter.Close();

            //Has to be closed after reading the inputs...
            this.Close();

            Process game;
            try
            {
                game = Process.Start("X3AP.exe", "/skipintro /noabout /faststart 478");
            }
            catch(Win32Exception)
            {
                MessageBox.Show("X3:Albion Prelude wurde nicht gefunden.", "Fehler", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
                return;
            }

            Thread.Sleep(8000);
            LibraryMapper mapper = new LibraryMapper(game, "X3MP.dll");
            mapper.MapLibrary();
        }

        #endregion

        private System.Windows.Forms.Button connectButton;
        private TextBox IpField;
        private TextBox NameInput;
        private Label label1;
        private Label label2;
        private Label label3;
        private NumericUpDown PortInput;
        private Label label4;
        private Button closeButton;
    }
}


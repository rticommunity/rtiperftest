using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.Diagnostics;

// Includes Nini namespace (don't forget to add the reference in your project)
using Nini.Config;

namespace BasicApp
{
	/// <summary>
	/// Windows forms application that demonstrates some of Nini's 
	/// core features.
	/// </summary>
	public class MainForm : System.Windows.Forms.Form
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		private System.Windows.Forms.GroupBox loggingGroup;
		private System.Windows.Forms.Label logFileNameLabel;
		private System.Windows.Forms.TextBox logFileNameText;
		private System.Windows.Forms.TextBox maxFileSizeText;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button viewIniButton;
		private System.Windows.Forms.Button saveIniButton;
		private System.Windows.Forms.TextBox userEmailText;
		private System.Windows.Forms.Label userEmailLabel;
		private System.Windows.Forms.TextBox userNameText;
		private System.Windows.Forms.Label userNameLabel;
		private System.Windows.Forms.Button closeButton;
		private IConfigSource source = null;

		public MainForm ()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent ();

			LoadConfigs ();
		}

		/// <summary>
		/// Loads all configuration values into the UI controls.
		/// </summary>
		private void LoadConfigs ()
		{
			// Load the configuration source file
			source = new IniConfigSource (@"..\..\BasicApp.ini");

			// Set the config to the Logging section of the INI file.
			IConfig config = source.Configs["Logging"];

			// Load up some normal configuration values
			logFileNameText.Text = config.Get ("File Name");
			maxFileSizeText.Text = config.Get ("MaxFileSize");
			userNameText.Text = source.Configs["User"].Get ("Name");
			userEmailText.Text = source.Configs["User"].Get ("Email");
		}

		#region Required methods
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			Application.Run (new MainForm ());
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose (bool disposing)
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}
		#endregion

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.logFileNameLabel = new System.Windows.Forms.Label();
			this.loggingGroup = new System.Windows.Forms.GroupBox();
			this.userNameText = new System.Windows.Forms.TextBox();
			this.userNameLabel = new System.Windows.Forms.Label();
			this.saveIniButton = new System.Windows.Forms.Button();
			this.viewIniButton = new System.Windows.Forms.Button();
			this.maxFileSizeText = new System.Windows.Forms.TextBox();
			this.label1 = new System.Windows.Forms.Label();
			this.logFileNameText = new System.Windows.Forms.TextBox();
			this.userEmailText = new System.Windows.Forms.TextBox();
			this.userEmailLabel = new System.Windows.Forms.Label();
			this.closeButton = new System.Windows.Forms.Button();
			this.loggingGroup.SuspendLayout();
			this.SuspendLayout();
			// 
			// logFileNameLabel
			// 
			this.logFileNameLabel.Location = new System.Drawing.Point(16, 32);
			this.logFileNameLabel.Name = "logFileNameLabel";
			this.logFileNameLabel.Size = new System.Drawing.Size(88, 16);
			this.logFileNameLabel.TabIndex = 1;
			this.logFileNameLabel.Text = "Log File Name:";
			// 
			// loggingGroup
			// 
			this.loggingGroup.Controls.AddRange(new System.Windows.Forms.Control[] {
																					   this.closeButton,
																					   this.userNameText,
																					   this.userNameLabel,
																					   this.saveIniButton,
																					   this.viewIniButton,
																					   this.maxFileSizeText,
																					   this.label1,
																					   this.logFileNameText,
																					   this.logFileNameLabel,
																					   this.userEmailText,
																					   this.userEmailLabel});
			this.loggingGroup.Location = new System.Drawing.Point(16, 24);
			this.loggingGroup.Name = "loggingGroup";
			this.loggingGroup.Size = new System.Drawing.Size(440, 200);
			this.loggingGroup.TabIndex = 2;
			this.loggingGroup.TabStop = false;
			this.loggingGroup.Text = "IConfigs (Sections) in INI file";
			// 
			// userNameText
			// 
			this.userNameText.Location = new System.Drawing.Point(128, 120);
			this.userNameText.Name = "userNameText";
			this.userNameText.Size = new System.Drawing.Size(152, 20);
			this.userNameText.TabIndex = 12;
			this.userNameText.Text = "";
			// 
			// userNameLabel
			// 
			this.userNameLabel.Location = new System.Drawing.Point(16, 120);
			this.userNameLabel.Name = "userNameLabel";
			this.userNameLabel.Size = new System.Drawing.Size(88, 16);
			this.userNameLabel.TabIndex = 11;
			this.userNameLabel.Text = "User Name";
			// 
			// saveIniButton
			// 
			this.saveIniButton.Location = new System.Drawing.Point(312, 32);
			this.saveIniButton.Name = "saveIniButton";
			this.saveIniButton.Size = new System.Drawing.Size(112, 23);
			this.saveIniButton.TabIndex = 8;
			this.saveIniButton.Text = "Save Changes";
			this.saveIniButton.Click += new System.EventHandler(this.saveIniButton_Click);
			// 
			// viewIniButton
			// 
			this.viewIniButton.Location = new System.Drawing.Point(312, 72);
			this.viewIniButton.Name = "viewIniButton";
			this.viewIniButton.Size = new System.Drawing.Size(112, 23);
			this.viewIniButton.TabIndex = 7;
			this.viewIniButton.Text = "View INI File";
			this.viewIniButton.Click += new System.EventHandler(this.viewIniButton_Click);
			// 
			// maxFileSizeText
			// 
			this.maxFileSizeText.Location = new System.Drawing.Point(128, 72);
			this.maxFileSizeText.Name = "maxFileSizeText";
			this.maxFileSizeText.Size = new System.Drawing.Size(152, 20);
			this.maxFileSizeText.TabIndex = 4;
			this.maxFileSizeText.Text = "";
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(16, 72);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(104, 16);
			this.label1.TabIndex = 3;
			this.label1.Text = "Log File Max Size:";
			// 
			// logFileNameText
			// 
			this.logFileNameText.Location = new System.Drawing.Point(128, 32);
			this.logFileNameText.Name = "logFileNameText";
			this.logFileNameText.Size = new System.Drawing.Size(152, 20);
			this.logFileNameText.TabIndex = 2;
			this.logFileNameText.Text = "";
			// 
			// userEmailText
			// 
			this.userEmailText.Location = new System.Drawing.Point(128, 160);
			this.userEmailText.Name = "userEmailText";
			this.userEmailText.Size = new System.Drawing.Size(152, 20);
			this.userEmailText.TabIndex = 10;
			this.userEmailText.Text = "";
			// 
			// userEmailLabel
			// 
			this.userEmailLabel.Location = new System.Drawing.Point(16, 160);
			this.userEmailLabel.Name = "userEmailLabel";
			this.userEmailLabel.Size = new System.Drawing.Size(88, 16);
			this.userEmailLabel.TabIndex = 9;
			this.userEmailLabel.Text = "User Email";
			// 
			// closeButton
			// 
			this.closeButton.Location = new System.Drawing.Point(312, 120);
			this.closeButton.Name = "closeButton";
			this.closeButton.Size = new System.Drawing.Size(112, 23);
			this.closeButton.TabIndex = 13;
			this.closeButton.Text = "Close";
			this.closeButton.Click += new System.EventHandler(this.closeButton_Click);
			// 
			// MainForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(472, 245);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.loggingGroup});
			this.Name = "MainForm";
			this.Text = "Nini C# BasicApp";
			this.loggingGroup.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		/// <summary>
		/// Handles saving the file.
		/// </summary>
		private void saveIniButton_Click (object sender, System.EventArgs e)
		{
			source.Configs["Logging"].Set ("File Name", logFileNameText.Text);
			source.Configs["Logging"].Set ("MaxFileSize", maxFileSizeText.Text);

			source.Configs["User"].Set ("Name", userNameText.Text);
			source.Configs["User"].Set ("Email", userEmailText.Text);

			// Save the INI file
			source.Save ();
		}

		/// <summary>
		/// Loads up the INI file in whatever editor is the default.
		/// </summary>
		private void viewIniButton_Click (object sender, System.EventArgs e)
		{
			Process.Start("notepad.exe", @"..\..\BasicApp.ini");
		}

		/// <summary>
		/// Handles the close button event.
		/// </summary>
		private void closeButton_Click (object sender, System.EventArgs e)
		{
			this.Close ();
		}

	}
}

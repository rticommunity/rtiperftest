Imports System.Diagnostics
Imports Nini.Config

Public Class MainForm
	Inherits System.Windows.Forms.Form

#Region " Windows Form Designer generated code "

	'Form overrides dispose to clean up the component list.
	Protected Overloads Overrides Sub Dispose(ByVal disposing As Boolean)
		If disposing Then
			If Not (components Is Nothing) Then
				components.Dispose()
			End If
		End If
		MyBase.Dispose(disposing)
	End Sub

	'Required by the Windows Form Designer
	Private components As System.ComponentModel.IContainer

	'NOTE: The following procedure is required by the Windows Form Designer
	'It can be modified using the Windows Form Designer.  
	'Do not modify it using the code editor.
	Friend WithEvents loggingGroup As System.Windows.Forms.GroupBox
	Friend WithEvents userNameText As System.Windows.Forms.TextBox
	Friend WithEvents userNameLabel As System.Windows.Forms.Label
	Friend WithEvents saveIniButton As System.Windows.Forms.Button
	Friend WithEvents viewIniButton As System.Windows.Forms.Button
	Friend WithEvents maxFileSizeText As System.Windows.Forms.TextBox
	Friend WithEvents label1 As System.Windows.Forms.Label
	Friend WithEvents logFileNameText As System.Windows.Forms.TextBox
	Friend WithEvents logFileNameLabel As System.Windows.Forms.Label
	Friend WithEvents userEmailText As System.Windows.Forms.TextBox
	Friend WithEvents userEmailLabel As System.Windows.Forms.Label
	Friend WithEvents closeButton As System.Windows.Forms.Button
	<System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
Me.loggingGroup = New System.Windows.Forms.GroupBox()
Me.userNameText = New System.Windows.Forms.TextBox()
Me.userNameLabel = New System.Windows.Forms.Label()
Me.saveIniButton = New System.Windows.Forms.Button()
Me.viewIniButton = New System.Windows.Forms.Button()
Me.maxFileSizeText = New System.Windows.Forms.TextBox()
Me.label1 = New System.Windows.Forms.Label()
Me.logFileNameText = New System.Windows.Forms.TextBox()
Me.logFileNameLabel = New System.Windows.Forms.Label()
Me.userEmailText = New System.Windows.Forms.TextBox()
Me.userEmailLabel = New System.Windows.Forms.Label()
Me.closeButton = New System.Windows.Forms.Button()
Me.loggingGroup.SuspendLayout()
Me.SuspendLayout()
'
'loggingGroup
'
Me.loggingGroup.Controls.AddRange(New System.Windows.Forms.Control() {Me.closeButton, Me.userNameText, Me.userNameLabel, Me.saveIniButton, Me.viewIniButton, Me.maxFileSizeText, Me.label1, Me.logFileNameText, Me.logFileNameLabel, Me.userEmailText, Me.userEmailLabel})
Me.loggingGroup.Location = New System.Drawing.Point(8, 16)
Me.loggingGroup.Name = "loggingGroup"
Me.loggingGroup.Size = New System.Drawing.Size(440, 200)
Me.loggingGroup.TabIndex = 3
Me.loggingGroup.TabStop = False
Me.loggingGroup.Text = "IConfigs (Sections) in INI file"
'
'userNameText
'
Me.userNameText.Location = New System.Drawing.Point(128, 120)
Me.userNameText.Name = "userNameText"
Me.userNameText.Size = New System.Drawing.Size(152, 20)
Me.userNameText.TabIndex = 12
Me.userNameText.Text = ""
'
'userNameLabel
'
Me.userNameLabel.Location = New System.Drawing.Point(16, 120)
Me.userNameLabel.Name = "userNameLabel"
Me.userNameLabel.Size = New System.Drawing.Size(88, 16)
Me.userNameLabel.TabIndex = 11
Me.userNameLabel.Text = "User Name"
'
'saveIniButton
'
Me.saveIniButton.Location = New System.Drawing.Point(312, 32)
Me.saveIniButton.Name = "saveIniButton"
Me.saveIniButton.Size = New System.Drawing.Size(112, 23)
Me.saveIniButton.TabIndex = 8
Me.saveIniButton.Text = "Save Changes"
'
'viewIniButton
'
Me.viewIniButton.Location = New System.Drawing.Point(312, 72)
Me.viewIniButton.Name = "viewIniButton"
Me.viewIniButton.Size = New System.Drawing.Size(112, 23)
Me.viewIniButton.TabIndex = 7
Me.viewIniButton.Text = "View INI File"
'
'maxFileSizeText
'
Me.maxFileSizeText.Location = New System.Drawing.Point(128, 72)
Me.maxFileSizeText.Name = "maxFileSizeText"
Me.maxFileSizeText.Size = New System.Drawing.Size(152, 20)
Me.maxFileSizeText.TabIndex = 4
Me.maxFileSizeText.Text = ""
'
'label1
'
Me.label1.Location = New System.Drawing.Point(16, 72)
Me.label1.Name = "label1"
Me.label1.Size = New System.Drawing.Size(104, 16)
Me.label1.TabIndex = 3
Me.label1.Text = "Log File Max Size:"
'
'logFileNameText
'
Me.logFileNameText.Location = New System.Drawing.Point(128, 32)
Me.logFileNameText.Name = "logFileNameText"
Me.logFileNameText.Size = New System.Drawing.Size(152, 20)
Me.logFileNameText.TabIndex = 2
Me.logFileNameText.Text = ""
'
'logFileNameLabel
'
Me.logFileNameLabel.Location = New System.Drawing.Point(16, 32)
Me.logFileNameLabel.Name = "logFileNameLabel"
Me.logFileNameLabel.Size = New System.Drawing.Size(88, 16)
Me.logFileNameLabel.TabIndex = 1
Me.logFileNameLabel.Text = "Log File Name:"
'
'userEmailText
'
Me.userEmailText.Location = New System.Drawing.Point(128, 160)
Me.userEmailText.Name = "userEmailText"
Me.userEmailText.Size = New System.Drawing.Size(152, 20)
Me.userEmailText.TabIndex = 10
Me.userEmailText.Text = ""
'
'userEmailLabel
'
Me.userEmailLabel.Location = New System.Drawing.Point(16, 160)
Me.userEmailLabel.Name = "userEmailLabel"
Me.userEmailLabel.Size = New System.Drawing.Size(88, 16)
Me.userEmailLabel.TabIndex = 9
Me.userEmailLabel.Text = "User Email"
'
'closeButton
'
Me.closeButton.Location = New System.Drawing.Point(312, 120)
Me.closeButton.Name = "closeButton"
Me.closeButton.Size = New System.Drawing.Size(112, 23)
Me.closeButton.TabIndex = 13
Me.closeButton.Text = "Close"
'
'MainForm
'
Me.AutoScaleBaseSize = New System.Drawing.Size(5, 13)
Me.ClientSize = New System.Drawing.Size(456, 237)
Me.Controls.AddRange(New System.Windows.Forms.Control() {Me.loggingGroup})
Me.Name = "MainForm"
Me.Text = "Nini VB.NET BasicApp"
Me.loggingGroup.ResumeLayout(False)
Me.ResumeLayout(False)

	End Sub

#End Region

	Private source As IniConfigSource

	' Constructor
	Public Sub New()
		MyBase.New()

		'This call is required by the Windows Form Designer.
		InitializeComponent()

		LoadConfigs()

	End Sub

	' Loads all configuration values into the UI controls.
	Public Sub LoadConfigs()
		' Load the configuration source file
		source = New IniConfigSource("..\BasicApp.ini")

		' Set the config to the Logging section of the INI file.
		Dim config As IConfig
		config = source.Configs("Logging")

		' Load up some normal configuration values
		logFileNameText.Text = config.Get("File Name")
		maxFileSizeText.Text = config.Get("MaxFileSize")
		userNameText.Text = source.Configs("User").Get("Name")
		userEmailText.Text = source.Configs("User").Get("Email")
	End Sub

	' Handles saving the file.
	Private Sub saveIniButton_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles saveIniButton.Click
		source.Configs("Logging").Set("File Name", logFileNameText.Text)
		source.Configs("Logging").Set("MaxFileSize", maxFileSizeText.Text)

		source.Configs("User").Set("Name", userNameText.Text)
		source.Configs("User").Set("Email", userEmailText.Text)

		' Save the INI file
		source.Save()
	End Sub

	' Loads up the INI file in whatever editor is the default.
	Private Sub viewIniButton_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles viewIniButton.Click
		Process.Start("notepad.exe", "..\BasicApp.ini")
	End Sub

	' Handles the close button event.
	Private Sub closeButton_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles closeButton.Click
		Me.Close()
	End Sub
End Class

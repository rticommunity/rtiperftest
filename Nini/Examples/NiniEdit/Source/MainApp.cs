#region Copyright
//
// Nini Configuration Project.
// Copyright (C) 2006 Brent R. Matzelle.  All rights reserved.
//
// This software is published under the terms of the MIT X11 license, a copy of 
// which has been included with this distribution in the LICENSE.txt file.
// 
#endregion

using System;

namespace NiniEdit
{
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class MainApp
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static int Main (string[] args)
		{
			Editor config = new Editor (args);
			
			try
			{
				config.ProcessArgs ();
				return 0;
			}
			catch (Exception ex)
			{
				Console.WriteLine ("ERROR: " + ex.Message);
				return 1;
			}
		}
	}
}

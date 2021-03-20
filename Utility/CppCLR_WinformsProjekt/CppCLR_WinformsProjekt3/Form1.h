/*Form1.h - WinForms for the Utility 
 */

#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <string>
#include <map>
#include <unordered_map>
#include <chrono>

#include "File_parser.h"

#pragma once

namespace CppCLRWinformsProjekt {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace fparser;
	/// <summary>
	/// Zusammenfassung f�r Form1
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
		Form1(void)
		{
			InitializeComponent();
			//
			//TODO: Konstruktorcode hier hinzuf�gen.
			//
			//
			this->progressBar1->Value = 0;
			this->progressBar1->Step = 1;
			this->progressBar1->Maximum = 200;
			timer1->Interval = 100;
		}

	protected:
		/// <summary>
		/// Verwendete Ressourcen bereinigen.
		/// </summary>
		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^ button1;
	private: System::Windows::Forms::ProgressBar^ progressBar1;
	private: System::Windows::Forms::DataGridView^ dataGridView1;
	private: System::Windows::Forms::Button^ button2;
	private: System::Windows::Forms::TextBox^ textBox1;
	private: System::Windows::Forms::Timer^ timer1;
	private: System::Windows::Forms::Button^ button3;
	private: System::ComponentModel::BackgroundWorker^ backgroundWorker1;
	private: System::ComponentModel::IContainer^ components;
	private: System::String^ current_filename;
	private: fparser::PairCounter *parse_results = nullptr;

	protected:

	private:
		/// <summary>
		/// Erforderliche Designervariable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Erforderliche Methode f�r die Designerunterst�tzung.
		/// Der Inhalt der Methode darf nicht mit dem Code-Editor ge�ndert werden.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->progressBar1 = (gcnew System::Windows::Forms::ProgressBar());
			this->dataGridView1 = (gcnew System::Windows::Forms::DataGridView());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->textBox1 = (gcnew System::Windows::Forms::TextBox());
			this->timer1 = (gcnew System::Windows::Forms::Timer(this->components));
			this->button3 = (gcnew System::Windows::Forms::Button());
			this->backgroundWorker1 = (gcnew System::ComponentModel::BackgroundWorker());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridView1))->BeginInit();
			this->SuspendLayout();
			// 
			// button1
			// 
			this->button1->Location = System::Drawing::Point(204, 86);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(75, 23);
			this->button1->TabIndex = 0;
			this->button1->Text = L"parse";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &Form1::button1_Click);
			// 
			// progressBar1
			// 
			this->progressBar1->Location = System::Drawing::Point(204, 422);
			this->progressBar1->Name = L"progressBar1";
			this->progressBar1->Size = System::Drawing::Size(478, 23);
			this->progressBar1->TabIndex = 1;
			// 
			// dataGridView1
			// 
			this->dataGridView1->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->dataGridView1->Location = System::Drawing::Point(204, 152);
			this->dataGridView1->Name = L"dataGridView1";
			this->dataGridView1->RowHeadersWidth = 51;
			this->dataGridView1->RowTemplate->Height = 24;
			this->dataGridView1->Size = System::Drawing::Size(478, 215);
			this->dataGridView1->TabIndex = 2;
			this->dataGridView1->CellContentClick += gcnew System::Windows::Forms::DataGridViewCellEventHandler(this, &Form1::dataGridView1_CellContentClick);
			// 
			// button2
			// 
			this->button2->Location = System::Drawing::Point(73, 32);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(75, 23);
			this->button2->TabIndex = 3;
			this->button2->Text = L"browse";
			this->button2->UseVisualStyleBackColor = true;
			this->button2->Click += gcnew System::EventHandler(this, &Form1::button2_Click);
			// 
			// textBox1
			// 
			this->textBox1->Location = System::Drawing::Point(204, 33);
			this->textBox1->Name = L"textBox1";
			this->textBox1->Size = System::Drawing::Size(478, 22);
			this->textBox1->TabIndex = 4;
			this->textBox1->Text = L"No File Selected";
			this->textBox1->TextChanged += gcnew System::EventHandler(this, &Form1::textBox1_TextChanged);
			// 
			// timer1
			// 
			this->timer1->Tick += gcnew System::EventHandler(this, &Form1::timer1_Tick);
			// 
			// button3
			// 
			this->button3->Location = System::Drawing::Point(73, 422);
			this->button3->Name = L"button3";
			this->button3->Size = System::Drawing::Size(75, 23);
			this->button3->TabIndex = 5;
			this->button3->Text = L"Cancel";
			this->button3->UseVisualStyleBackColor = true;
			this->button3->Click += gcnew System::EventHandler(this, &Form1::button3_Click);
			// 
			// backgroundWorker1
			// 
			this->backgroundWorker1->WorkerReportsProgress = true;
			this->backgroundWorker1->WorkerSupportsCancellation = true;
			this->backgroundWorker1->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &Form1::backgroundWorker1_DoWork);
			this->backgroundWorker1->ProgressChanged += gcnew System::ComponentModel::ProgressChangedEventHandler(this, &Form1::backgroundWorker1_ProgressChanged);
			this->backgroundWorker1->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &Form1::backgroundWorker1_RunWorkerCompleted);
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(885, 488);
			this->Controls->Add(this->button3);
			this->Controls->Add(this->textBox1);
			this->Controls->Add(this->button2);
			this->Controls->Add(this->dataGridView1);
			this->Controls->Add(this->progressBar1);
			this->Controls->Add(this->button1);
			this->Name = L"Form1";
			this->Text = L"Utility";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridView1))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

	/**
	 * Implementation of a Button to parse the textfile
	*/
	private: System::Void parse_gui_code() {

	}

	private: System::Void button1_Click(System::Object^ sender, System::EventArgs^ e) {
		if (!backgroundWorker1->IsBusy) {
			// disable buttons to prevent multiple files from being processed at once
			disable_buttons();

			// clear the existing datagridview
			dataGridView1->Columns->Clear();

			//----- Parse the text file and populate the table in the data grid -----
			if (current_filename == "")
			{
				MessageBox::Show("select the text file");
				enable_buttons();
				return;
			}
			progressBar1->Maximum = 100.;

			using namespace Runtime::InteropServices;
			const char* filepath_temp =
				(const char*)(Marshal::StringToHGlobalAnsi(current_filename)).ToPointer();
			std::string filepath = filepath_temp;
			Marshal::FreeHGlobal(IntPtr((void*)filepath_temp));

			if (parse_results != nullptr) {
				parse_results->clear();
			}
			else {
				parse_results = new fparser::PairCounter();
			}

			// Start the background worker
			backgroundWorker1->RunWorkerAsync();
		}
	}

    /**
     * Implementation of a File Dialog
    */

	private: System::Void button2_Click(System::Object^ sender, System::EventArgs^ e) {

		//----- FILE OPEN DIALOG BOX -----
		OpenFileDialog^ SelectFileDialog = gcnew OpenFileDialog();

		String^ Filename;
		SelectFileDialog->InitialDirectory = "c:\\";
		SelectFileDialog->Filter = "txt files (*.txt)|*.txt|All files (*.*)|*.*";
		SelectFileDialog->FilterIndex = 1;				//(First entry is 1, not 0)
		SelectFileDialog->RestoreDirectory = true;

		if (SelectFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			// 
			Filename = SelectFileDialog->FileName;
			textBox1->Text = Filename;
			current_filename = Filename;
		}
	}
	private: System::Void dataGridView1_CellContentClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellEventArgs^ e) {
	}
	private: System::Void timer1_Tick(System::Object^ sender, System::EventArgs^ e) {

		timer1->Stop();
	}
	private: System::Void button3_Click(System::Object^ sender, System::EventArgs^ e) {
		backgroundWorker1->CancelAsync();
		return;
	}

	private: System::Void disable_buttons() {
		button1->Enabled = false;
		button2->Enabled = false;
		textBox1->Enabled = false;
	}

	private: System::Void enable_buttons() {
		button1->Enabled = true;
		button2->Enabled = true;
		textBox1->Enabled = true;
	}

	private: System::Void backgroundWorker1_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e) {

		using namespace Runtime::InteropServices;
		const char* filepath_temp =
			(const char*)(Marshal::StringToHGlobalAnsi(current_filename)).ToPointer();
		std::string filepath = filepath_temp;
		Marshal::FreeHGlobal(IntPtr((void*)filepath_temp));

		int error_status = fparser::parser(filepath, *parse_results, backgroundWorker1);

		if (error_status == 1) {
			e->Cancel = true;
		}
	}

	private: System::Void backgroundWorker1_ProgressChanged(System::Object^ sender, System::ComponentModel::ProgressChangedEventArgs^ e) {
		progressBar1->Value = e->ProgressPercentage;
	}

	private: System::Void backgroundWorker1_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e) {
		if (e->Cancelled) {
			MessageBox::Show("The parsing of file " + current_filename + " is cancelled");
			progressBar1->Value = 0.0;
		}
		else {
			if (parse_results->size() > 0) {
				progressBar1->Value = 100.0;
				dataGridView1->DataSource = nullptr;			//Clear the grid if necessary
				dataGridView1->Columns->Clear();

				dataGridView1->AutoGenerateColumns = false;
				dataGridView1->ColumnCount = 2;
				dataGridView1->Columns[0]->Name = "Word";
				dataGridView1->Columns[1]->Name = "Count";

				int i = 0;
				for (const auto m : *parse_results) {
					dataGridView1->Rows->Add();
					String^ key = gcnew String(m.first.c_str());
					dataGridView1->Rows[i]->Cells[0]->Value = key;
					dataGridView1->Rows[i++]->Cells[1]->Value = m.second.ToString();
				}
			}
			else {
				MessageBox::Show("The parsing of file " + current_filename + " was unsuccessful");
			}
		}
		enable_buttons();
	}

	private: System::Void textBox1_TextChanged(System::Object^ sender, System::EventArgs^ e) {
		current_filename = textBox1->Text;
	}
};
}



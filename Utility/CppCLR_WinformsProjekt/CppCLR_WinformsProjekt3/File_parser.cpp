/* File_parser.cpp - File parser
 */
/***
 *  Implementation of a parser to parse the text file.
 *  @file File_parser.cpp
 *  @author Yashas Nagaraj Udupa
 *  @version 05/03/2021
*/

#include "pch.h"
#include "File_parser.h"

/**
 * Helper function to parse the text fle
 *
 * @param values - WindowsTextBox and ProgressBar to parsing and updating progress.
 * @return Table which consists of pair of words and their occurences.
 */

int fparser::parser(const std::string& filename, fparser::PairCounter& pair_counter, BackgroundWorker^ worker)
{
	// exception thrown when there is an error in opening the file 
	int error_state = 0;
	try
	{
		// Find the size of the text file
		std::ifstream stream(filename.c_str());
		stream.seekg(0, stream.end);
		auto file_size = stream.tellg();
		stream.seekg(0, stream.beg);
		long long int max_file_size = file_size;
		long long int current_file_postion = 0;

		std::string current_line;
		std::string current_word;

		// Implementation of a parse-logic
		while (std::getline(stream, current_line, '\n'))
		{
			std::vector<std::string> word_array;
			std::istringstream linestream(current_line);

			if (worker != nullptr && worker->CancellationPending) {
				pair_counter.clear();
				error_state = 1;
				break;
			}
			while (std::getline(linestream, current_word, ' '))
			{
				if (current_word == "" || current_word == " " || current_word == "  ")
					continue;
				//if (current_word.find(":") != -1) // ':' as a delimter to create table
				//{
				//	pair_counter.push_back(std::make_pair(current_word, 1));
				//	continue;
				//}
				word_array.push_back(current_word);
			}

			// Count the number of occurences
			for (const auto word : word_array)
			{
				bool word_exists = false;
				for (auto& pair : pair_counter)
				{
					if (pair.first == word)
					{
						pair.second = pair.second + 1;
						word_exists = true;
						break;
					}
				}
				// Sorting according to the number of occurences was computationally 
				// expensive therefore, another method is used.
				if (!word_exists)
				{
					pair_counter.push_back(std::make_pair(word, 1));
				}
			}
			// Update the progress bar for every row analysis
			auto current_char_count = current_line.size() + 1;
			current_file_postion += current_char_count;

			int current_progress = (current_file_postion * 100) / max_file_size;
			if (current_progress > 100) current_progress = 100;

			worker->ReportProgress(current_progress);
		}
	}
	catch (std::ifstream::failure e) {
		std::cerr << "Exception opening/reading/closing file\n";
		error_state = 2;
	}
	return error_state;
}
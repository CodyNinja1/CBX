#include <iostream>
#include <fstream>
#include "GbxReader.h"

std::ifstream OpenFile(const char* Filename)
{
	using std::ifstream;
	ifstream Stream(Filename, std::ios::binary);
	return Stream;
}

int main(int argc, const char* argv[])
{
	using namespace CBX;
	if (argc == 1)
	{
		std::cerr << "No file provided.\n";
		return 1;
	}
	else
	{
		auto Stream = OpenFile(argv[1]);
		if (!Stream.is_open())
		{
			std::cerr << "Invalid file provided: " << argv[1] << "\n";
			return 1;
		}
		else
		{
			try
			{
				auto Parser = GbxReader(Stream);
				std::cout << "ClassId: " << (void*)Parser.ClassId << "\n";
				std::cout << "UserData Size: " << Parser.UserDataSize << "\n";
				std::cout << "HeaderChunks (" << Parser.HeaderChunkCount << "):\n";

				for (size_t Idx = 0; Idx < Parser.HeaderChunkCount; Idx++)
				{
					auto Chunk = &Parser.HeaderChunks[Idx];
					std::cout << "ChunkId " << (void*)Chunk->Id << " (" << Chunk->Size << "B)" << (Chunk->Heavy ? " (Heavy)" : "") << std::endl;
				}
			}
			catch (std::exception Exception)
			{
				std::cerr << "Parser throwed an error: " << Exception.what() << "\n";
			}
		}
	}
}
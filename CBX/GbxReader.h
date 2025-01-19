#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace CBX
{
	enum ByteFormat
	{
		Bytes,
		Text
	};

	enum Compression
	{
		Compressed,
		Uncompressed
	};

	enum Appendix
	{
		R,
		E
	};

	class Chunk 
	{
		public:
			unsigned int Id, Size;
			char* Data = nullptr;
			bool Heavy;
			std::istringstream* DataStream = nullptr;

			Chunk(unsigned int Id, unsigned int Size, bool Heavy, char* Data)
			{
				this->Id = Id;
				this->Size = Size;
				this->Data = Data;
				this->Heavy = Heavy;

				if (this->Data)
				{
					std::string s(this->Data, this->Size);
					std::istringstream Stream(s);
					DataStream = &Stream;
				}
			}

			std::string ReadString(unsigned long Size)
			{
				std::string Result(Size, '\0');
				if (DataStream->read(&Result[0], Size))
				{
					return Result;
				}
				return std::string(1, '\0');
			}

			template <typename T>
			T Read()
			{
				T Result = -1;
				if (DataStream->read(reinterpret_cast<char*>(&Result), sizeof(T)))
				{
					return Result;
				}
				return -1;
			}

			void SetData(char* DataIn)
			{
				this->Data = DataIn;

				if (this->Data)
				{
					std::string s(this->Data, this->Size);
					std::istringstream Stream(s);
					DataStream = &Stream;
				}
			}
	};

	class GbxReader
	{
		void InitBuffer(std::ifstream& FileStream)
		{
			FileStream.seekg(0, std::ios::end);
			FileSize = FileStream.tellg();
			FileStream.seekg(0, std::ios::beg);
		}

		std::string ReadString(unsigned long long Size)
		{
			std::string Result(Size, '\0');
			if (FileStream->read(&Result[0], Size))
			{
				return Result;
			}
			return std::string(1, '\0');
		}

		template <typename T>
		T Read()
		{
			T Result = -1;
			if (FileStream->read(reinterpret_cast<char*>(&Result), sizeof(T)))
			{
				return Result;
			}
			return -1;
		}

	public:
		unsigned long FileSize = 0;
		std::ifstream* FileStream = nullptr;

		unsigned short int GbxVer = -1;
		ByteFormat Format;
		Compression RefTableCompresison;
		Compression BodyCompression;
		Appendix _A;
		unsigned int ClassId;
		unsigned int UserDataSize;
		unsigned int HeaderChunkCount;
		std::vector<Chunk> HeaderChunks;

		GbxReader(std::ifstream& FileStream)
		{
			InitBuffer(FileStream);
			this->FileStream = &FileStream;

			auto GBXMagic = ReadString(3);

			if (GBXMagic != "GBX")
			{
				std::cerr << "Not gbx file.\n";
				throw new std::exception();
			}
			else
			{
				ParseHeader();
			}
		}

		void ParseHeader()
		{
			GbxVer = Read<unsigned short int>();

			if (GbxVer >= 3)
			{
				Format = ReadString(1) == "B" ? Bytes : Text;
				RefTableCompresison = ReadString(1) == "C" ? Compressed : Uncompressed;
				BodyCompression = ReadString(1) == "C" ? Compressed : Uncompressed;
				if (GbxVer >= 4) _A = ReadString(1) == "R" ? R : E;

				ClassId = Read<unsigned int>();

				if (GbxVer >= 6)
				{
					UserDataSize = Read<unsigned int>(); // honestly useless

					HeaderChunkCount = Read<unsigned int>();
					for (size_t Idx = 0; Idx < HeaderChunkCount; Idx++)
					{
						auto Id = Read<unsigned int>();
						auto Size = Read<unsigned int>();
						HeaderChunks.push_back(Chunk(Id, Size & 0x7FFFFFFF, (Size & 0x80000000) == 0x80000000, nullptr));
					}

					for (size_t Idx = 0; Idx < HeaderChunkCount; Idx++)
					{
						char* ChunkData = new(std::nothrow) char[HeaderChunks[Idx].Size];
						if (!ChunkData)
						{
							std::cout << "tried to allocate a shit load of data: " << HeaderChunks[Idx].Size << std::endl;
							delete[] ChunkData;
						}
						FileStream->read(ChunkData, HeaderChunks[Idx].Size);
						HeaderChunks[Idx].SetData(ChunkData);
						delete[] ChunkData;
					}
				}
			}
		}
	};
}
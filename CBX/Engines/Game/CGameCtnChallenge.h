#pragma once
#include "../../GbxReader.h"

namespace CBX
{
	class CGameCtnChallenge
	{
		CGameCtnChallenge(GbxReader& Reader)
		{
			if (Reader.ClassId != 0x03043000)
			{
				throw new std::exception();
			}
		}
	};
}


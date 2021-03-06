// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
//  ixen@copyhandler.com
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation;
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================
#ifndef __TFAKEFILESERIALIZER_H__
#define __TFAKEFILESERIALIZER_H__

#include "ISerializer.h"

namespace serializer
{
	class LIBSERIALIZER_API TFakeFileSerializer : public ISerializer
	{
	public:
		explicit TFakeFileSerializer(const chcore::TSmartPath& rPath);
		virtual ~TFakeFileSerializer();

		TFakeFileSerializer(const TFakeFileSerializer& rSrc) = delete;
		TFakeFileSerializer& operator=(const TFakeFileSerializer& rSrc) = delete;

		chcore::TSmartPath GetLocation() const override;
		ISerializerContainerPtr GetContainer(const string::TString& strContainerName) override;
		void Flush() override;

	private:
		chcore::TSmartPath m_pathFileSerializer;
	};

	typedef std::shared_ptr<TFakeFileSerializer> TFakeFileSerializerPtr;
}

#endif

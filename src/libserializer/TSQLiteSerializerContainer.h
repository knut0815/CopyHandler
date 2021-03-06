// ============================================================================
//  Copyright (C) 2001-2014 by Jozef Starosczyk
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
#ifndef __TSQLITESERIALIZERCONTAINER_H__
#define __TSQLITESERIALIZERCONTAINER_H__

#include "ISerializerRowReader.h"
#include "ISerializerContainer.h"
#include <boost/pool/poolfwd.hpp>
#include "TSQLiteColumnDefinition.h"
#include "TSQLiteDatabase.h"
#include "TSQLiteSerializerRowData.h"
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include "../liblogger/TLogger.h"
#include "libserializer.h"

namespace serializer
{
	class LIBSERIALIZER_API TSQLiteSerializerContainer : public ISerializerContainer
	{
	public:
		TSQLiteSerializerContainer(const string::TString& strName, const sqlite::TSQLiteDatabasePtr& spDB, TPlainStringPool& poolStrings, const logger::TLogFileDataPtr& spLogFileData);
		TSQLiteSerializerContainer(const TSQLiteSerializerContainer&) = delete;
		virtual ~TSQLiteSerializerContainer();

		TSQLiteSerializerContainer& operator=(const TSQLiteSerializerContainer&) = delete;

		IColumnsDefinition& GetColumnsDefinition() override;

		ISerializerRowData& GetRow(object_id_t oidRowID, bool bMarkAsAdded) override;
		void DeleteRow(object_id_t oidRowID) override;
		void DeleteRows(const TRemovedObjects& setObjects) override;

		ISerializerRowReaderPtr GetRowReader() override;

		void Flush();

	private:
		void FlushDeletions();
		boost::pool<>& GetPool();
		size_t CalculateRowMemorySize() const;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		TSQLiteColumnsDefinition m_tColumns;

		boost::pool<>* m_pPoolRows;

		typedef boost::container::flat_map<object_id_t, std::unique_ptr<TSQLiteSerializerRowData>> RowMap;	// maps row id to row data
		RowMap m_mapRows;

		boost::container::flat_set<object_id_t> m_setDeleteItems;

		string::TString m_strName;
		sqlite::TSQLiteDatabasePtr m_spDB;

		TPlainStringPool& m_poolStrings;

		logger::TLoggerPtr m_spLog;
#pragma warning(pop)
	};

	using TSQLiteSerializerContainerPtr = std::shared_ptr<TSQLiteSerializerContainer>;
}

#endif

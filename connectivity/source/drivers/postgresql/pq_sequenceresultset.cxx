/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  Effective License of whole file:
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *  Parts "Copyright by Sun Microsystems, Inc" prior to August 2011:
 *
 *    The Contents of this file are made available subject to the terms of
 *    the GNU Lesser General Public License Version 2.1
 *
 *    Copyright: 2000 by Sun Microsystems, Inc.
 *
 *    Contributor(s): Joerg Budischewski
 *
 *  All parts contributed on or after August 2011:
 *
 *    This Source Code Form is subject to the terms of the Mozilla Public
 *    License, v. 2.0. If a copy of the MPL was not distributed with this
 *    file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 ************************************************************************/


#include "pq_sequenceresultset.hxx"
#include "pq_sequenceresultsetmetadata.hxx"

#include <connectivity/dbexception.hxx>

using com::sun::star::sdbc::XResultSetMetaData;

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;

namespace pq_sdbc_driver
{

void SequenceResultSet::checkClosed()
    throw ( com::sun::star::sdbc::SQLException,
            com::sun::star::uno::RuntimeException )
{
    // we never close :o)
}


Any SequenceResultSet::getValue( sal_Int32 columnIndex )
{
    m_wasNull = ! m_data[m_row][columnIndex -1 ].hasValue();
    return m_data[m_row][columnIndex -1 ];
}

SequenceResultSet::SequenceResultSet(
    const ::rtl::Reference< RefCountedMutex > & mutex,
    const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > &owner,
    const Sequence< OUString > &colNames,
    const Sequence< Sequence< Any > > &data,
    const Reference< com::sun::star::script::XTypeConverter > & tc,
    const ColumnMetaDataVector *pVec) :
    BaseResultSet( mutex, owner, data.getLength(), colNames.getLength(),tc ),
    m_data(data ),
    m_columnNames( colNames )
{
    if( pVec )
    {
        m_meta = new SequenceResultSetMetaData( *pVec, m_columnNames.getLength() );
    }
}

SequenceResultSet::~SequenceResultSet()
{

}

void SequenceResultSet::close(  )
    throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    // a noop
}

Reference< XResultSetMetaData > SAL_CALL SequenceResultSet::getMetaData(  )
    throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    if( ! m_meta.is() )
    {
        // Oh no, not again
        throw ::com::sun::star::sdbc::SQLException(
            "pq_sequenceresultset: no meta supported ", *this,
        // I did not find "IM001" in a specific standard,
        // but it seems to be used by other systems (such as ODBC)
        // and some parts of LibreOffice special-case it.
            OUString( "IM001" ), 1, Any() );
    }
    return m_meta;
}


sal_Int32 SAL_CALL SequenceResultSet::findColumn(
    const OUString& columnName )
    throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    // no need to guard, as all members are readonly !
    for( int i = 0 ;i < m_fieldCount ; i ++ )
    {
        if( columnName == m_columnNames[i] )
        {
            return i+1;
        }
    }
    ::dbtools::throwInvalidColumnException( columnName, *this );
    assert(false);
    return 0; // Never reached
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_EDITENG_WRITINGMODEITEM_HXX
#define INCLUDED_EDITENG_WRITINGMODEITEM_HXX

#include <com/sun/star/text/WritingMode.hpp>
#include <svl/intitem.hxx>
#include <editeng/editengdllapi.h>

// class SvxWritingModeItem ----------------------------------------------

class EDITENG_DLLPUBLIC SvxWritingModeItem : public SfxUInt16Item
{
public:
    static SfxPoolItem* CreateDefault();

    SvxWritingModeItem( css::text::WritingMode eValue /*= css::text::WritingMode_LR_TB*/,
                sal_uInt16 nWhich /*= SDRATTR_TEXTDIRECTION*/ );
    virtual ~SvxWritingModeItem();

    SvxWritingModeItem& operator=( const SvxWritingModeItem& rItem );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const override;
    virtual SvStream&       Store(SvStream & rStrm, sal_uInt16 nIVer) const override;
    virtual sal_uInt16          GetVersion( sal_uInt16 nFileVersion ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper * = nullptr ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
};

#endif // INCLUDED_EDITENG_WRITINGMODEITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

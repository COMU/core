/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* MWAWCalcImportFilter: Sets up the filter, and calls DocumentCollector
 * to do the actual filtering
 *
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/Reference.h>
#include <cppuhelper/supportsservice.hxx>

#include <libmwaw/libmwaw.hxx>
#include <libodfgen/libodfgen.hxx>

#include "MWAWCalcImportFilter.hxx"

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XComponentContext;

static bool handleEmbeddedMWAWGraphicObject(const librevenge::RVNGBinaryData &data, OdfDocumentHandler *pHandler,  const OdfStreamType streamType)
{
    OdgGenerator exporter;
    exporter.addDocumentHandler(pHandler, streamType);
    return MWAWDocument::decodeGraphic(data, &exporter);
}

static bool handleEmbeddedMWAWSpreadsheetObject(const librevenge::RVNGBinaryData &data, OdfDocumentHandler *pHandler,  const OdfStreamType streamType)
{
    OdsGenerator exporter;
    exporter.addDocumentHandler(pHandler, streamType);
    return MWAWDocument::decodeSpreadsheet(data, &exporter);
}

bool MWAWCalcImportFilter::doImportDocument(librevenge::RVNGInputStream &rInput, OdsGenerator &rGenerator, utl::MediaDescriptor &)
{
    return MWAWDocument::MWAW_R_OK == MWAWDocument::parse(&rInput, &rGenerator);
}

bool MWAWCalcImportFilter::doDetectFormat(librevenge::RVNGInputStream &rInput, OUString &rTypeName)
{
    rTypeName.clear();

    MWAWDocument::Type docType = MWAWDocument::MWAW_T_UNKNOWN;
    MWAWDocument::Kind docKind = MWAWDocument::MWAW_K_UNKNOWN;
    const MWAWDocument::Confidence confidence = MWAWDocument::isFileFormatSupported(&rInput, docType, docKind);

    if (confidence == MWAWDocument::MWAW_C_EXCELLENT)
    {
        switch (docKind)
        {
        case MWAWDocument::MWAW_K_DATABASE:
            switch (docType)
            {
            case MWAWDocument::MWAW_T_CLARISWORKS:
                rTypeName = "calc_ClarisWorks";
                break;
            case MWAWDocument::MWAW_T_MICROSOFTWORKS:
                rTypeName = "calc_Mac_Works";
                break;
            default:
                rTypeName = "MWAW_Database";
                break;
            }
            break;
        case MWAWDocument::MWAW_K_SPREADSHEET:
            switch (docType)
            {
            case MWAWDocument::MWAW_T_CLARISRESOLVE:
                rTypeName = "calc_Claris_Resolve";
                break;
            case MWAWDocument::MWAW_T_CLARISWORKS:
                rTypeName = "calc_ClarisWorks";
                break;
            case MWAWDocument::MWAW_T_MICROSOFTWORKS:
                rTypeName = "calc_Mac_Works";
                break;
            default:
                rTypeName = "MWAW_Spreadsheet";
                break;
            }
            break;
        default:
            break;
        }
    }

    return !rTypeName.isEmpty();
}

void MWAWCalcImportFilter::doRegisterHandlers(OdsGenerator &rGenerator)
{
    rGenerator.registerEmbeddedObjectHandler("image/mwaw-odg", &handleEmbeddedMWAWGraphicObject);
    rGenerator.registerEmbeddedObjectHandler("image/mwaw-ods", &handleEmbeddedMWAWSpreadsheetObject);
}

OUString MWAWCalcImportFilter_getImplementationName()
throw (RuntimeException)
{
    return OUString("com.sun.star.comp.Calc.MWAWCalcImportFilter");
}

Sequence< OUString > SAL_CALL MWAWCalcImportFilter_getSupportedServiceNames()
throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ImportFilter";
    pArray[1] =  "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

Reference< XInterface > SAL_CALL MWAWCalcImportFilter_createInstance(const Reference< XComponentContext > &rContext)
throw(Exception)
{
    return static_cast<cppu::OWeakObject *>(new MWAWCalcImportFilter(rContext));
}

// XServiceInfo
OUString SAL_CALL MWAWCalcImportFilter::getImplementationName()
throw (RuntimeException, std::exception)
{
    return MWAWCalcImportFilter_getImplementationName();
}
sal_Bool SAL_CALL MWAWCalcImportFilter::supportsService(const OUString &rServiceName)
throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}
Sequence< OUString > SAL_CALL MWAWCalcImportFilter::getSupportedServiceNames()
throw (RuntimeException, std::exception)
{
    return MWAWCalcImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

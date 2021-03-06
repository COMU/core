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


#include <xmlsecurity/xmlsignaturehelper.hxx>
#include <xmlsecurity/documentsignaturehelper.hxx>
#include "xsecctl.hxx"

#include "xmlsignaturehelper2.hxx"

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/datetime.hxx>

#include <xmloff/attrlist.hxx>

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>

#include <tools/date.hxx>
#include <tools/time.hxx>
#include <comphelper/ofopxmlhelper.hxx>
#include <comphelper/sequence.hxx>

#define TAG_DOCUMENTSIGNATURES  "document-signatures"
#define NS_DOCUMENTSIGNATURES   "http://openoffice.org/2004/documentsignatures"
#define NS_DOCUMENTSIGNATURES_ODF_1_2 "urn:oasis:names:tc:opendocument:xmlns:digitalsignature:1.0"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

XMLSignatureHelper::XMLSignatureHelper( const uno::Reference< uno::XComponentContext >& rxCtx)
    : mxCtx(rxCtx), mbODFPre1_2(false)
{
    mpXSecController = new XSecController(rxCtx);
    mxSecurityController = mpXSecController;
    mbError = false;
}

XMLSignatureHelper::~XMLSignatureHelper()
{
}

bool XMLSignatureHelper::Init()
{
    DBG_ASSERT( !mxSEInitializer.is(), "XMLSignatureHelper::Init - mxSEInitializer already set!" );
    DBG_ASSERT( !mxSecurityContext.is(), "XMLSignatureHelper::Init - mxSecurityContext already set!" );

    mxSEInitializer = com::sun::star::xml::crypto::SEInitializer::create( mxCtx );

    if ( mxSEInitializer.is() )
        mxSecurityContext = mxSEInitializer->createSecurityContext( OUString() );

    return mxSecurityContext.is();
}

void XMLSignatureHelper::SetStorage(
    const Reference < css::embed::XStorage >& rxStorage,
    const OUString& sODFVersion)
{
    DBG_ASSERT( !mxUriBinding.is(), "SetStorage - UriBinding already set!" );
    mxUriBinding = new UriBindingHelper( rxStorage );
    DBG_ASSERT(rxStorage.is(), "SetStorage - empty storage!");
    mbODFPre1_2 = DocumentSignatureHelper::isODFPre_1_2(sODFVersion);
}


void XMLSignatureHelper::SetStartVerifySignatureHdl( const Link<LinkParamNone*,bool>& rLink )
{
    maStartVerifySignatureHdl = rLink;
}


void XMLSignatureHelper::StartMission()
{
    if ( !mxUriBinding.is() )
        mxUriBinding = new UriBindingHelper();

    mpXSecController->startMission( mxUriBinding, mxSecurityContext );
}

void XMLSignatureHelper::EndMission()
{
    mpXSecController->endMission();
}

sal_Int32 XMLSignatureHelper::GetNewSecurityId()
{
    return mpXSecController->getNewSecurityId();
}

void XMLSignatureHelper::SetX509Certificate(
        sal_Int32 nSecurityId,
        const OUString& ouX509IssuerName,
        const OUString& ouX509SerialNumber,
        const OUString& ouX509Cert)
{
    mpXSecController->setX509Certificate(
        nSecurityId,
        ouX509IssuerName,
        ouX509SerialNumber,
        ouX509Cert);
}

void XMLSignatureHelper::SetDateTime( sal_Int32 nSecurityId, const ::Date& rDate, const tools::Time& rTime )
{
    css::util::DateTime stDateTime = ::DateTime(rDate, rTime).GetUNODateTime();
    mpXSecController->setDate( nSecurityId, stDateTime );
}

void XMLSignatureHelper::SetDescription(sal_Int32 nSecurityId, const OUString& rDescription)
{
    mpXSecController->setDescription(nSecurityId, rDescription);
}

void XMLSignatureHelper::AddForSigning( sal_Int32 nSecurityId, const OUString& uri, const OUString& objectURL, bool bBinary )
{
    mpXSecController->signAStream( nSecurityId, uri, objectURL, bBinary );
}


uno::Reference<xml::sax::XWriter> XMLSignatureHelper::CreateDocumentHandlerWithHeader(
    const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& xOutputStream )
{
    /*
     * get SAX writer component
     */
    uno::Reference< lang::XMultiComponentFactory > xMCF( mxCtx->getServiceManager() );
    uno::Reference< xml::sax::XWriter > xSaxWriter = xml::sax::Writer::create(mxCtx);

    /*
     * connect XML writer to output stream
     */
    xSaxWriter->setOutputStream( xOutputStream );

    /*
     * write the xml context for signatures
     */
    OUString tag_AllSignatures(TAG_DOCUMENTSIGNATURES);

    SvXMLAttributeList *pAttributeList = new SvXMLAttributeList();
    OUString sNamespace;
    if (mbODFPre1_2)
        sNamespace = NS_DOCUMENTSIGNATURES;
    else
        sNamespace = NS_DOCUMENTSIGNATURES_ODF_1_2;

    pAttributeList->AddAttribute(
        ATTR_XMLNS,
        sNamespace);

    xSaxWriter->startDocument();
    xSaxWriter->startElement(
        tag_AllSignatures,
        uno::Reference< com::sun::star::xml::sax::XAttributeList > (pAttributeList));

    return xSaxWriter;
}

void XMLSignatureHelper::CloseDocumentHandler( const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler )
{
    OUString tag_AllSignatures(TAG_DOCUMENTSIGNATURES);
    xDocumentHandler->endElement( tag_AllSignatures );
    xDocumentHandler->endDocument();
}

void XMLSignatureHelper::ExportSignature(
    const uno::Reference< xml::sax::XDocumentHandler >& xDocumentHandler,
    const SignatureInformation& signatureInfo )
{
    XSecController::exportSignature(xDocumentHandler, signatureInfo);
}

bool XMLSignatureHelper::CreateAndWriteSignature( const uno::Reference< xml::sax::XDocumentHandler >& xDocumentHandler )
{
    mbError = false;

    /*
     * create a signature listener
     */

    /*
     * configure the signature creation listener
     */

    /*
     * write signatures
     */
    if ( !mpXSecController->WriteSignature( xDocumentHandler ) )
    {
        mbError = true;
    }

    /*
     * clear up the signature creation listener
     */

    return !mbError;
}

bool XMLSignatureHelper::ReadAndVerifySignature( const com::sun::star::uno::Reference< com::sun::star::io::XInputStream >& xInputStream )
{
    mbError = false;

    DBG_ASSERT(xInputStream.is(), "input stream missing");

    /*
     * prepare ParserInputSrouce
     */
    xml::sax::InputSource aParserInput;
    aParserInput.aInputStream = xInputStream;

    /*
     * get SAX parser component
     */
    uno::Reference< xml::sax::XParser > xParser = xml::sax::Parser::create(mxCtx);

    /*
     * create a signature reader
     */
    uno::Reference< xml::sax::XDocumentHandler > xHandler
        = mpXSecController->createSignatureReader( );

    /*
     * create a signature listener
     */
    ImplXMLSignatureListener* pSignatureListener = new ImplXMLSignatureListener(
                                                    LINK( this, XMLSignatureHelper, SignatureCreationResultListener ),
                                                    LINK( this, XMLSignatureHelper, SignatureVerifyResultListener ),
                                                    LINK( this, XMLSignatureHelper, StartVerifySignatureElement ) );

    /*
     * configure the signature verify listener
     */

    /*
     * setup the connection:
     * Parser -> SignatureListener -> SignatureReader
     */
    pSignatureListener->setNextHandler(xHandler);
    xParser->setDocumentHandler( pSignatureListener );

    /*
     * parser the stream
     */
    try
    {
        xParser->parseStream( aParserInput );
    }
    catch( xml::sax::SAXParseException& )
    {
        mbError = true;
    }
    catch( xml::sax::SAXException& )
    {
        mbError = true;
    }
    catch( com::sun::star::io::IOException& )
    {
        mbError = true;
    }
    catch( uno::Exception& )
    {
        mbError = true;
    }

    /*
     * clear up the connection
     */
    pSignatureListener->setNextHandler( nullptr );

    /*
     * clear up the signature verify listener
     */

    /*
     * release the signature reader
     */
    mpXSecController->releaseSignatureReader( );

    return !mbError;
}

SignatureInformation XMLSignatureHelper::GetSignatureInformation( sal_Int32 nSecurityId ) const
{
    return mpXSecController->getSignatureInformation( nSecurityId );
}

SignatureInformations XMLSignatureHelper::GetSignatureInformations() const
{
    return mpXSecController->getSignatureInformations();
}

uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > XMLSignatureHelper::GetSecurityEnvironment()
{
    return (mxSecurityContext.is()?(mxSecurityContext->getSecurityEnvironment()): uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment >());
}

IMPL_LINK_TYPED( XMLSignatureHelper, SignatureCreationResultListener, XMLSignatureCreationResult&, rResult, void )
{
    maCreationResults.insert( maCreationResults.begin() + maCreationResults.size(), rResult );
    if ( rResult.nSignatureCreationResult != com::sun::star::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED )
        mbError = true;
}

IMPL_LINK_TYPED( XMLSignatureHelper, SignatureVerifyResultListener, XMLSignatureVerifyResult&, rResult, void )
{
    maVerifyResults.insert( maVerifyResults.begin() + maVerifyResults.size(), rResult );
    if ( rResult.nSignatureVerifyResult != com::sun::star::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED )
        mbError = true;
}

IMPL_LINK_NOARG_TYPED( XMLSignatureHelper, StartVerifySignatureElement, LinkParamNone*, void )
{
    if ( !maStartVerifySignatureHdl.IsSet() || maStartVerifySignatureHdl.Call(nullptr) )
    {
        sal_Int32 nSignatureId = mpXSecController->getNewSecurityId();
        mpXSecController->addSignature( nSignatureId );
    }
}

namespace
{
bool lcl_isSignatureType(const beans::StringPair& rPair)
{
    return rPair.First == "Type" && rPair.Second == "http://schemas.openxmlformats.org/package/2006/relationships/digital-signature/signature";
}
}

bool XMLSignatureHelper::ReadAndVerifySignatureStorage(const uno::Reference<embed::XStorage>& xStorage)
{
    sal_Int32 nOpenMode = embed::ElementModes::READ;
    uno::Reference<embed::XStorage> xSubStorage = xStorage->openStorageElement("_rels", nOpenMode);
    uno::Reference<io::XInputStream> xRelStream(xSubStorage->openStreamElement("origin.sigs.rels", nOpenMode), uno::UNO_QUERY);
    uno::Sequence< uno::Sequence<beans::StringPair> > aRelationsInfo;
    aRelationsInfo = comphelper::OFOPXMLHelper::ReadRelationsInfoSequence(xRelStream, "origin.sigs.rels", mxCtx);

    for (const uno::Sequence<beans::StringPair>& rRelation : aRelationsInfo)
    {
        auto aRelation = comphelper::sequenceToContainer< std::vector<beans::StringPair> >(rRelation);
        if (std::find_if(aRelation.begin(), aRelation.end(), lcl_isSignatureType) != aRelation.end())
        {
            std::vector<beans::StringPair>::iterator it = std::find_if(aRelation.begin(), aRelation.end(), [](const beans::StringPair& rPair) { return rPair.First == "Target"; });
            if (it != aRelation.end())
            {
                uno::Reference<io::XInputStream> xInputStream(xStorage->openStreamElement(it->Second, nOpenMode), uno::UNO_QUERY);
                if (!ReadAndVerifySignatureStorageStream(xInputStream))
                    return false;
            }
        }
    }

    return true;
}

bool XMLSignatureHelper::ReadAndVerifySignatureStorageStream(const css::uno::Reference<css::io::XInputStream>& xInputStream)
{
    mbError = false;

    // Create the input source.
    xml::sax::InputSource aParserInput;
    aParserInput.aInputStream = xInputStream;

    // Create the sax parser.
    uno::Reference<xml::sax::XParser> xParser = xml::sax::Parser::create(mxCtx);

    // Create the signature reader.
    uno::Reference<xml::sax::XDocumentHandler> xHandler = mpXSecController->createSignatureReader(embed::StorageFormats::OFOPXML);

    // Create the signature listener.
    ImplXMLSignatureListener* pSignatureListener = new ImplXMLSignatureListener(
        LINK(this, XMLSignatureHelper, SignatureCreationResultListener),
        LINK(this, XMLSignatureHelper, SignatureVerifyResultListener),
        LINK(this, XMLSignatureHelper, StartVerifySignatureElement));
    uno::Reference<xml::sax::XDocumentHandler> xSignatureListener(pSignatureListener);

    // Parser -> signature listener -> signature reader.
    pSignatureListener->setNextHandler(xHandler);
    xParser->setDocumentHandler(xSignatureListener);

    // Parse the stream.
    try
    {
        xParser->parseStream(aParserInput);
    }
    catch(const uno::Exception& rException)
    {
        SAL_WARN("xmlsecurity.helper", "XMLSignatureHelper::ReadAndVerifySignatureStorageStream: " << rException.Message);
    }

    pSignatureListener->setNextHandler(nullptr);
    mpXSecController->releaseSignatureReader();

    return !mbError;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

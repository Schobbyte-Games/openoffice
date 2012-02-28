/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#include <SerfCallbacks.hxx>

#include <SerfSession.hxx>
#include <SerfRequestProcessor.hxx>

using namespace http_dav_ucp;

extern "C" apr_status_t Serf_ConnectSetup( apr_socket_t *skt,
                                           serf_bucket_t **read_bkt,
                                           serf_bucket_t **write_bkt,
                                           void *setup_baton,
                                           apr_pool_t *pool )
{
    SerfSession* pSerfSession = static_cast< SerfSession* >( setup_baton );
    return pSerfSession->setupSerfConnection( skt,
                                              read_bkt,
                                              write_bkt,
                                              pool );
}

extern "C" apr_status_t Serf_Credentials( char **username, 
                                          char **password,
                                          serf_request_t *request,
                                          void *baton,
                                          int code, 
                                          const char *authn_type,
                                          const char *realm,
                                          apr_pool_t *pool )
{
    SerfRequestProcessor* pReqProc = static_cast< SerfRequestProcessor* >( baton );
    return pReqProc->provideSerfCredentials( username, 
                                             password,
                                             request,
                                             code,
                                             authn_type,
                                             realm,
                                             pool );
}

extern "C" apr_status_t Serf_CertificationValidation( void *data,
                                                      int failures,
                                                      const serf_ssl_certificate_t *cert )
{
    SerfSession* pSerfSession = static_cast< SerfSession* >( data );
    return pSerfSession->verifySerfCertificate( failures, 
                                                cert );
}

extern "C" apr_status_t Serf_SetupRequest( serf_request_t *request,
                                           void *setup_baton,
                                           serf_bucket_t **req_bkt,
                                           serf_response_acceptor_t *acceptor,
                                           void **acceptor_baton,
                                           serf_response_handler_t *handler,
                                           void **handler_baton,
                                           apr_pool_t * pool )
{
    SerfRequestProcessor* pReqProc = static_cast< SerfRequestProcessor* >( setup_baton );
    return pReqProc->setupSerfRequest( request, 
                                       req_bkt,
                                       acceptor,
                                       acceptor_baton,
                                       handler,
                                       handler_baton,
                                       pool );
}

extern "C" serf_bucket_t* Serf_AcceptResponse( serf_request_t *request,
                                               serf_bucket_t *stream,
                                               void *acceptor_baton,
                                               apr_pool_t *pool )
{
    SerfRequestProcessor* pReqProc = static_cast< SerfRequestProcessor* >( acceptor_baton );
    return pReqProc->acceptSerfResponse( request,
                                         stream,
                                         pool );
}

extern "C" apr_status_t Serf_HandleResponse( serf_request_t *request,
                                             serf_bucket_t *response,
                                             void *handler_baton,
                                             apr_pool_t *pool )
{
    SerfRequestProcessor* pReqProc = static_cast< SerfRequestProcessor* >( handler_baton );
    return pReqProc->handleSerfResponse( request,
                                         response,
                                         pool );
}


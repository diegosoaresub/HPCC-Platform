/*##############################################################################

    Copyright (C) 2011 HPCC Systems.

    All rights reserved. This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
############################################################################## */

#ifndef _SOAPBIND_HPP__
#define _SOAPBIND_HPP__

#ifndef esp_http_decl
    #define esp_http_decl
#endif


//Jlib
#include "jliball.hpp"

//SCM Interfaces
#include "esp.hpp"
#include "soapesp.hpp"

//ESP Core
#include "espthread.hpp"

//ESP Bindings
#include "SOAP/Platform/soapmessage.hpp"

#include "espbinding.hpp"
#include "http/platform/httpbinding.hpp"


class esp_http_decl CSoapComplexType : public CInterface,
    implements IRpcSerializable
{
protected:
    unsigned clvalue_;
    unsigned msg_id_;
    void    *thunk_;
public:
    IMPLEMENT_IINTERFACE;

    CSoapComplexType() : clvalue_(0), msg_id_(0), thunk_(NULL) { }

    void setClientValue(unsigned val){clvalue_=val;}
    unsigned getClientValue(){return clvalue_;}

    void setMessageId(unsigned val){msg_id_=val;}
    unsigned getMessageId(){return msg_id_;}

    void setThunkHandle(void * val){thunk_=val;}
    void * getThunkHandle(){return thunk_;}

    virtual void appendContent(IEspContext* ctx, MemoryBuffer& buffer, StringBuffer& mimetype);
    virtual void serializeJSONStruct(IEspContext* ctx, StringBuffer& s, const char *name);
    virtual void serializeStruct(IEspContext * ctx, StringBuffer & buffer, const char * rootname=NULL);
    virtual void serializeItem(IEspContext* ctx, StringBuffer& s, const char *name);
    virtual void serializeAttributes(IEspContext* ctx, StringBuffer& s)=0;
    virtual void serializeContent(IEspContext* ctx, StringBuffer& buffer, IProperties **pprops=NULL) = 0;
    virtual void serialize(IEspContext * ctx, StringBuffer & buffer, const char * rootname=NULL)
    {
        serializeStruct(ctx, buffer, rootname);
    }

    virtual const char *getNsURI()=0;
    virtual const char *getNsPrefix()=0;
    virtual const char *getRootName()=0;
};

class esp_http_decl CSoapResponseBinding : public CSoapComplexType,
    implements IRpcResponseBinding,
    implements IEspResponse
{
private:
    RpcMessageState state_;
    StringBuffer redirectUrl_;
    Owned<IMultiException> exceptions_;

public:
    IMPLEMENT_IINTERFACE;

    CSoapResponseBinding()
    {
        state_=RPC_MESSAGE_OK;
        exceptions_.setown(MakeMultiException("CSoapResponseBinding"));
    }

    void setClientValue(unsigned val){clvalue_=val;}
    unsigned getClientValue(){return clvalue_;}

    void setMessageId(unsigned val){msg_id_=val;}
    unsigned getMessageId(){return msg_id_;}

    void setThunkHandle(void * val){thunk_=val;}
    void * getThunkHandle(){return thunk_;}

    void setRpcState(RpcMessageState state)
    {
        state_=state;
    }

    RpcMessageState getRpcState(){return state_;}

    virtual void setRedirectUrl(const char * url)
    {
        redirectUrl_.clear().append(url);
    }

    const char *getRedirectUrl()                   { return redirectUrl_.str();    }
    const IMultiException& getExceptions() { return *exceptions_;          }
    void  noteException(IException& e)     { exceptions_->append(e);          }
    void handleExceptions(IMultiException *me, const char *serv, const char *meth);
};


class esp_http_decl CSoapRequestBinding : public CSoapComplexType,
    implements IRpcRequestBinding,
    implements IEspRequest
{
private:
    StringBuffer url_;
    StringBuffer proxy_;
    StringBuffer userid_;
    StringBuffer password_;
    StringBuffer realm_;

public:
    IMPLEMENT_IINTERFACE;

    CSoapRequestBinding(){}

    void setClientValue(unsigned val){clvalue_=val;}
    unsigned getClientValue(){return clvalue_;}

    void setMessageId(unsigned val){msg_id_=val;}
    unsigned getMessageId(){return msg_id_;}

    void setThunkHandle(void * val){thunk_=val;}
    void * getThunkHandle(){return thunk_;}

    void setUrl(const char *url){url_.clear().append(url);} 
    const char * getUrl(){return url_.str();}

    void setProxyAddress(const char *proxy){proxy_.clear().append(proxy);}  
    const char * getProxyAddress(){return proxy_.str();}

    void setUserId(const char *userid){userid_.clear().append(userid);} 
    const char * getUserId(){return userid_.str();}

    void setPassword(const char *password){password_.clear().append(password);} 
    const char * getPassword(){return password_.str();}

    void setRealm(const char *realm){realm_.clear().append(realm);} 
    const char * getRealm(){return realm_.str();}

    void post(const char *proxy, const char* url, IRpcResponseBinding& response, const char *action=NULL);

    void post(IRpcResponseBinding& response)
    {
        post(getProxyAddress(), getUrl(), response);
    }

    virtual void serialize(IRpcMessage& rpc)
    {
        throw MakeStringException(-1,"Internal error: umimplmented function called: CSoapRequestBinding::serialize()");
    }
};


class esp_http_decl CSoapBinding : public CEspBinding
{
public:
    CSoapBinding();
    virtual ~CSoapBinding();

    virtual const char * getRpcType();
    virtual const char * getTransportType();

    virtual int processRequest(IRpcMessage* rpc_call, IRpcMessage* rpc_response);
};


typedef enum http_soap_log_level_ {hsl_none, hsl_all} http_soap_log_level;

class esp_http_decl CHttpSoapBinding : public CSoapBinding, public EspHttpBinding
{
public:
    CHttpSoapBinding();
    CHttpSoapBinding(IPropertyTree* cfg, const char *bindname=NULL, const char *procname=NULL, http_soap_log_level level=hsl_none);
    virtual ~CHttpSoapBinding();

    http_soap_log_level log_level_;

    virtual const char * getTransportType();
    virtual int onSoapRequest(CHttpRequest* request, CHttpResponse* response);
    virtual void setHSLogLevel(http_soap_log_level level){log_level_=level;}
    virtual int onGetNavEvent(IEspContext & context, IHttpMessage * req, IHttpMessage * resp){return 0;}
    IEspContainer* queryContainer() { return m_container; }
    virtual int HandleSoapRequest(CHttpRequest* request, CHttpResponse* response);

};

void SetHTTPErrorStatus(int ErrorCode,CHttpResponse* response);

#endif //_SOAPBIND_HPP__

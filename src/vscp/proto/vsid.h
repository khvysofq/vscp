#ifndef VSCP_PROTO_VSID_H_
#define VSCP_PROTO_VSID_H_

#include "vscp/proto/protobasicincludes.h"

namespace vscp {

  // The vsid class encapsulates and provides parsing help for vsids. A vsid
  // consists of three parts: the node, the domain and the resource, e.g.:
  //
  // node@domain/resource
  //
  // The node and resource are both optional. A valid vsid is defined to have
  // a domain. A bare vsid is defined to not have a resource and a full vsid
  // *does* have a resource.

  class Vsid {
  public:
    explicit Vsid();
    explicit Vsid(const std::string& vsid_string);
    explicit Vsid(const std::string& node_name,
      const std::string& domain_name,
      const std::string& resource_name);
    ~Vsid();

    const std::string & node() const { return node_name_; }
    const std::string & domain() const { return domain_name_; }
    const std::string & resource() const { return resource_name_; }

    std::string Str() const;
    Vsid BareVsid() const;

    bool IsEmpty() const;
    bool IsValid() const;
    bool IsBare() const;
    bool IsFull() const;

    bool BareEquals(const Vsid& other) const;
    void CopyFrom(const Vsid& vsid);
    bool operator==(const Vsid& other) const;
    bool operator!=(const Vsid& other) const { return !operator==(other); }

    bool operator<(const Vsid& other) const { return Compare(other) < 0; };
    bool operator>(const Vsid& other) const { return Compare(other) > 0; };

    int Compare(const Vsid & other) const;

  private:
    void ValidateOrReset();

    static std::string PrepNode(const std::string& node, bool* valid);
    static char PrepNodeAscii(char ch, bool* valid);
    static std::string PrepResource(const std::string& start, bool* valid);
    static char PrepResourceAscii(char ch, bool* valid);
    static std::string PrepDomain(const std::string& domain, bool* valid);
    static void PrepDomain(const std::string& domain,
      std::string* buf, bool* valid);
    static void PrepDomainLabel(
      std::string::const_iterator start, std::string::const_iterator end,
      std::string* buf, bool* valid);
    static char PrepDomainLabelAscii(char ch, bool *valid);

    std::string node_name_;
    std::string domain_name_;
    std::string resource_name_;
  };
}; // namespace vscp


#endif
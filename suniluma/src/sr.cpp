#include "../include/simulator.h"
#include "../include/helper.h"
#include<iostream>
#include<string>
#include<vector>
using namespace std;

#define BUFFER 1000
#define MSG_SIZE 20
#define INTERRUPT 12.0

string buffer[BUFFER];
int seqno[BUFFER];
float timer[BUFFER];
int acks[BUFFER];

int index = 0;
int acked = 0;

int win_start = 0;
int win_end = 0;

struct pkt gen_pkt(string message, int seqnum) {
    struct pkt p;
    p.seqnum = seqnum;
    p.acknum = seqnum;
    for(int i = 0; i < MSG_SIZE; i++) {
        p.payload[i] = (char)message[i];
    }
    p.checksum = 0;
    for(int i = 0; i < MSG_SIZE; i++) {
        p.checksum += (int)p.payload[i];
    }
    p.checksum += p.seqnum + p.acknum;
    return p;
}

bool validate_checksum(struct pkt p) {
    int checksum = 0;
    for(int i = 0; i < MSG_SIZE; i++) {
        checksum += (int)p.payload[i];
    }
    checksum += p.seqnum + p.acknum;
    if(checksum == p.checksum) {
        return true;
    } else {
        return false;
    }
}

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
    buffer[index++] = (char*)message.data;
    for(int i = win_start; i < win_end && i < BUFFER; i++) {
        if(buffer[i] != "" && acks[i] == -1) {
            timer[i] = get_sim_time();
            tolayer3(0,gen_pkt(buffer[i],seqno[i]));
            acks[i] = 0;
        }
    }

    for(int i = win_start; i < win_end && i < BUFFER; i++) {
        if(timer[i] > 0.0f && ((get_sim_time() - timer[i]) > INTERRUPT)) {
            timer[i] = get_sim_time();
            tolayer3(0,gen_pkt(buffer[i],seqno[i]));
            acks[i] = 0;
        }
    }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
    if(validate_checksum(packet)) {
        timer[packet.acknum] = -1.0f;
        buffer[packet.acknum] = "";
        acks[packet.acknum] = 1;
    }
    for(int i = win_start; i < win_end && i < BUFFER; i++) {
        if(timer[i] == -1.0f) {
            win_start = i+1;
        } else {
            break;
        }
    }
    win_end = win_start + getwinsize();
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    /*
    for(int i = win_start; i < win_end && i < BUFFER; i++) {
        if(buffer[i] == "") {
            return;
        } else {
            if(i == win_start) {
                starttimer(0,INTERRUPT);
            }
            tolayer3(0,gen_pkt(buffer[i],seqno[i]));
        }

    }
    */
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    for(int i = 0; i < BUFFER; i++) {
        seqno[i] = i;
        timer[i] = 0.0f;
        acks[i] = -1;
    }
    win_end = getwinsize();
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */
int b_ack = 0;
struct pkt buff[BUFFER];
int b_acked[BUFFER];
/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    if(validate_checksum(packet)) {
        tolayer3(1,packet);
        buff[packet.seqnum] = packet;
        b_acked[packet.seqnum] = 1;
    }

    for(int i = b_ack; i < BUFFER && buff[i].seqnum != -1; i++) {
        if(b_acked[i] != 1) {
            return;
        } else {
            tolayer5(1,buff[i].payload);
            b_ack +=1;
        }
    }


}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    for(int i = 0; i < BUFFER; i++) {
        b_acked[i] = -1;
        buff[i].seqnum = -1;
        buff[i].acknum = -1;
        buff[i].checksum = -1;
        for(int j = 0; j < 20; j++) {
            buff[i].payload[j] = ((char *)"\0")[0];
            cout<<buff[i].payload[j];
        }
    }
}

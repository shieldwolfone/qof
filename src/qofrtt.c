/**
 ** qofrtt.c
 ** RTT estimation data structures and function prototypes for QoF
 **
 ** ------------------------------------------------------------------------
 ** Copyright (C) 2013      Brian Trammell. All Rights Reserved
 ** ------------------------------------------------------------------------
 ** Authors: Brian Trammell <brian@trammell.ch>
 ** ------------------------------------------------------------------------
 ** QoF is made available under the terms of the
 ** GNU General Public License (GPL) Version 2, June 1991
 ** ------------------------------------------------------------------------
 */

#define _YAF_SOURCE_
#include <qof/qofrtt.h>
#include <qof/yaftab.h>

#define QOF_RTT_DEBUG 0

static int qfWrapCompare(uint32_t a, uint32_t b) {
    return a == b ? 0 : ((a - b) & 0x80000000) ? -1 : 1;
}


static void qfRttSetAckWait(qfRttDir_t  *dir,
                            uint32_t    seq,
                            uint32_t    ms)
{
    dir->ackwait = 1;
    dir->ecrwait = 0;
    dir->lms = ms;
    dir->tsack = seq;
}


static void qfRttSetEcrWait(qfRttDir_t  *dir,
                            uint32_t    tsval,
                            uint32_t    ms)
{
    dir->ackwait = 0;
    dir->ecrwait = 1;
    dir->lms = ms;
    dir->tsack = tsval;
}

static int qfRttSample(qfRtt_t     *rtt)
{
    if (rtt->fwd.obs_ms && rtt->rev.obs_ms) {
        sstLinSmoothAdd(&rtt->val, rtt->fwd.obs_ms + rtt->rev.obs_ms);
#if QOF_RTT_DEBUG
        yfFlow_t *f = (yfFlow_t *)(((uint8_t*)rtt) - offsetof(yfFlow_t, rtt));
        fprintf(stderr,"%10llu fwd %4u rev %4u sample %4u last %4u min %4u n %4u\n",
                f->fid, rtt->fwd.obs_ms, rtt->rev.obs_ms,
                rtt->fwd.obs_ms + rtt->rev.obs_ms,
                rtt->val.val, rtt->val.min, rtt->val.n);
#endif
        return 1;
    } else {
        return 0;
    }
}


void qfRttSegment(qfRtt_t           *rtt,
                  uint32_t          seq,
                  uint32_t          ack,
                  uint32_t          tsval,
                  uint32_t          tsecr,
                  uint32_t          ms,
                  uint8_t           tcpflags,
                  unsigned          reverse)
{
    qfRttDir_t    *fdir, *rdir;
#if QOF_RTT_DEBUG
    char          *dirname;
#endif
    
    /* select which side we're looking at */
    if (reverse) {
        fdir = &rtt->rev;
        rdir = &rtt->fwd;
#if QOF_RTT_DEBUG
        dirname = "rev";
#endif
    } else {
        fdir = &rtt->fwd;
        rdir = &rtt->rev;
#if QOF_RTT_DEBUG
        dirname = "fwd";
#endif
    }

    if (fdir->ackwait && (tcpflags & YF_TF_ACK) &&
        qfWrapCompare(ack, fdir->tsack) >= 0)
    {
        /* got an ACK we were waiting for */
        fdir->obs_ms = ms - fdir->lms;
        if (qfRttSample(rtt)) {
#if QOF_RTT_DEBUG
            fprintf(stderr, "\ton %3s ack %u for seq %u (%u)\n", dirname,
                    ack, fdir->tsack, ack - fdir->tsack);
#endif
        }
        fdir->ackwait = 0;
        if (tsval) {
            qfRttSetEcrWait(rdir, tsval, ms);
        }
            
    } else if (fdir->ecrwait && qfWrapCompare(tsecr, fdir->tsack) >= 0) {
        /* got a TSECR we were waiting for */
        if ((ms - fdir->lms) > fdir->obs_ms) {
            /* Minimize measured RTT on TSECR samples */
            fdir->obs_ms = ms - fdir->lms;
            if (qfRttSample(rtt)) {
#if QOF_RTT_DEBUG
                fprintf(stderr, "\ton %s ecr %u for val %u (%u)\n", dirname,
                        tsecr, fdir->tsack, tsecr - fdir->tsack);
#endif
            }
        }
        fdir->ecrwait = 0;
        qfRttSetAckWait(rdir, seq, ms);
    } else if (!rdir->ackwait && !rdir->ecrwait) {
        qfRttSetAckWait(rdir, seq, ms);
    }
}
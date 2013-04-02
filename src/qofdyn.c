/**
 ** qofdyn.c
 ** TCP dynamics tracking data structures and algorithms for QoF
 **
 ** ------------------------------------------------------------------------
 ** Copyright (C) 2013      Brian Trammell. All Rights Reserved
 ** ------------------------------------------------------------------------
 ** Authors: Brian Trammell <brian@trammell.ch>
 ** ------------------------------------------------------------------------
 ** QoF is made available under the terms of the GNU Public License (GPL)
 ** Version 2, June 1991
 ** ------------------------------------------------------------------------
 */

#define _YAF_SOURCE_
#include <yaf/qofdyn.h>
#include <yaf/yaftab.h>

#define QF_DYN_DEBUG 1

/** Constant - 2^32 (for sequence number calculations) */
static const uint64_t k2e32 = 0x100000000ULL;

static const unsigned int kAlpha = 8;
static const unsigned int kSeqSamplePeriodMs = 1;

static size_t qf_dyn_bincap = 0;
static size_t qf_dyn_binscale = 0;
static size_t qf_dyn_ringcap = 0;

int qfSeqCompare(uint32_t a, uint32_t b) {
    return a == b ? 0 : ((a - b) & 0x80000000) ? -1 : 1;
}

/** Sequence number / time tuple */
struct qfSeqTime_st {
    uint32_t    ms;
    uint32_t    seq;
};

void qfSeqRingInit(qfSeqRing_t              *sr,
                   size_t                   capacity)
{
    /* zero structure */
    memset(sr, 0, sizeof(*sr));
    
    /* allocate bin array */
    sr->bincount = capacity;
    sr->bin = yg_slice_alloc0(sr->bincount * sizeof(qfSeqTime_t));
}

void qfSeqRingFree(qfSeqRing_t              *sr)
{
    if (sr->bin) yg_slice_free1(sr->bincount * sizeof(qfSeqTime_t), sr->bin);
}

void qfSeqRingAddSample(qfSeqRing_t         *sr,
                        uint32_t            seq,
                        uint32_t            ms)
{
    sr->seqcount++;
    
    /* overrun if we need to */
    if (((sr->head + 1) % sr->bincount) == sr->tail) {
        sr->tail++;
        if (sr->tail >= sr->bincount) sr->tail = 0;
        sr->overcount++;
#if QF_DYN_DEBUG
        fprintf(stderr, "rtts-over (%u) ", sr->overcount );
#endif
    }
    
    /* insert sample */
    sr->bin[sr->head].seq = seq;
    sr->bin[sr->head].ms = ms;
    sr->head++;
    if (sr->head >= sr->bincount) sr->head = 0;
}

uint32_t qfSeqRingRTT(qfSeqRing_t           *sr,
                      uint32_t              ack,
                      uint32_t              ms)
{
    uint32_t rtt;
    
    sr->ackcount++;
    
    while (sr->head != sr->tail &&
           qfSeqCompare(sr->bin[sr->tail].seq, ack-1) < 0)
    {
        sr->tail++;
        if (sr->tail >= sr->bincount) sr->tail = 0;
    }

    if (sr->head == sr->tail) return 0;

#if QF_DYN_DEBUG
    fprintf(stderr, "(seq %u @ %u ack %u @ %u) ",
            sr->bin[sr->tail].seq,
            sr->bin[sr->tail].ms,
            ack, ms);
#endif
    
    rtt = ms - sr->bin[sr->tail].ms;
    sr->tail++;
    if (sr->tail >= sr->bincount) sr->tail = 0;
    return rtt;
}

static size_t qfSeqRingAvail(qfSeqRing_t *sr)
{
    ssize_t occ = sr->head - sr->tail;
    if (occ < 0) occ += sr->bincount;
    return sr->bincount - occ;
}

static int qfDynHasSeqring(qfDyn_t *qd) { return qd->sr.bin != 0; }

static int qfDynHasSeqbin(qfDyn_t *qd) { return qd->sb.map.v != 0; }

static int qfDynSeqSampleP(qfDyn_t     *qd,
                           uint64_t     ms)
{
    /* don't sample without seqring */
    if (!qfDynHasSeqring(qd)) {
#if QF_DYN_DEBUG
        fprintf(stderr, "nortts-nil ");
#endif
        return 0;
    }
    
    /* don't sample higher than sample rate */
    if ((qd->sr.bin[qd->sr.head].ms + kSeqSamplePeriodMs) > ms)
    {
#if QF_DYN_DEBUG
        fprintf(stderr, "nortts-rate ");
#endif        
        return 0;
    }
    
    /* don't sample until we've waited out the sample period */
    if (qd->sr_skip < qd->sr_period) {
#if QF_DYN_DEBUG
        fprintf(stderr, "nortts-skip (%u of %u)", qd->sr_skip, qd->sr_period);
#endif
        qd->sr_skip++;
        return 0;
    }
    
    /* good to sample, calculate next period */
    if ((qd->dynflags & QF_DYN_SEQINIT) && (qd->dynflags & QF_DYN_ACKINIT)) {
        qd->sr_period = ((qd->fsn - qd->fan) / qd->mss) / qfSeqRingAvail(&qd->sr);
        if (qd->sr_period) qd->sr_period--;        
    } else {
        qd->sr_period = 0;
    }
    qd->sr_skip = 0;
    
    return 1;
}

void qfSeqBitsInit(qfSeqBits_t *sb, uint32_t capacity, uint32_t scale) {
    bimInit(&sb->map, capacity / scale);
    sb->scale = scale;
}

void qfSeqBitsFree(qfSeqBits_t *sb) {
    bimFree(&(sb->map));
}

int qfSeqBitsSegmentRtx(qfSeqBits_t *sb,
                        uint32_t aseq, uint32_t bseq)
{
    bimIntersect_t brv;
    uint32_t seqcap, maxseq;
    
    /* set seqbase on first segment */
    if (!sb->seqbase) {
        sb->seqbase = (aseq / sb->scale) * sb->scale;
    }
    
    /* move range forward if necessary to cover EOR */
    seqcap = sb->map.sz * k64Bits * sb->scale;
    maxseq = sb->seqbase + seqcap;
    while (qfSeqCompare(bseq, maxseq) > 0) {
        sb->lostseq_ct +=
            ((k64Bits - bimShiftDownAndCountSet(&sb->map)) * sb->scale);
        sb->seqbase += sb->scale * k64Bits;
        maxseq = sb->seqbase + seqcap;
    }
    
    /* entire range already seen (or rotated past): signal retransmission */
    if (qfSeqCompare(bseq, sb->seqbase) < 0) return 1;
    
    /* clip overlapping bottom of range */
    if (qfSeqCompare(aseq, sb->seqbase) < 0) aseq = sb->seqbase;

    /* set bits in the bitmap */
    brv = bimTestAndSetRange(&sb->map, (aseq - sb->seqbase)/sb->scale,
                                       (bseq - sb->seqbase)/sb->scale);
    
    /* translate intersection to retransmission detect */
    if (brv == BIM_PART || brv == BIM_FULL) {
        return 1;
    } else {
        return 0;
    }
}

static void qfDynRexmit(qfDyn_t     *qd,
                        uint32_t    seq,
                        uint64_t    ms)
{
    /* increment retransmit counter */
    qd->rtx_ct++;
}

static void qfDynCorrectRTT(qfDyn_t   *qd,
                           uint32_t    crtt)
{
    if (!qd->rtt_corr || (crtt < qd->rtt_corr)) qd->rtt_corr = crtt;
}

static void qfDynTrackRTT(qfDyn_t   *qd,
                          uint32_t  irtt)
{
    /* add correction factor to raw sample */
    irtt += qd->rtt_corr;
    
    if (qd->rtt_est) {
        /* moving average */
        qd->rtt_est = (qd->rtt_est * (kAlpha - 1) + irtt) / kAlpha;
    } else {
        /* take initial sample */
        qd->rtt_est = irtt;
    }

    /* minimize only if we have a correction term */
    if (qd->rtt_corr &&
        (!qd->rtt_min || (qd->rtt_est < qd->rtt_min)))
    {
        qd->rtt_min = qd->rtt_est;
        qd->dynflags = QF_DYN_RTTVALID; // FIXME might want to be less eager here
    }
}

void qfDynSetParams(size_t bincap, size_t binscale, size_t ringcap) {
    qf_dyn_bincap = bincap;
    qf_dyn_binscale = binscale;
    qf_dyn_ringcap = ringcap;
}

void qfDynFree(qfDyn_t      *qd)
{
#if QF_DYN_DEBUG
    fprintf(stderr, "qd free ring-seq %u ring-ack %u ring-over %u\n",
                    qd->sr.seqcount, qd->sr.ackcount, qd->sr.overcount);
#endif
   
//    qfSeqBinFree(&qd->sb);
    qfSeqRingFree(&qd->sr);
}

void qfDynSeq(qfDyn_t     *qd,
              uint32_t    seq,
              uint32_t    oct,
              uint32_t    ms)
{

#if QF_DYN_DEBUG
    fprintf(stderr, "qd ts %u seq [%10u - %10u] ", (uint32_t) (ms & UINT32_MAX), seq - oct, seq);
#endif
 
    /* short circuit on no octets */
    if (!oct) {
#if QF_DYN_DEBUG
        fprintf(stderr, "empty\n");
#endif
        return;
    }
    
    /* initialise if not yet done */
    if (!qd->dynflags & QF_DYN_SEQINIT) {        
        /* allocate structures */
        if (qf_dyn_ringcap) {
            qfSeqRingInit(&qd->sr, qf_dyn_ringcap);
        }

        if (qf_dyn_bincap) {
            qfSeqBitsInit(&qd->sb, qf_dyn_bincap, qf_dyn_binscale);
        }

        /* set ISN */
        qd->isn = seq - oct;
        
        /* force sequence number advance */
        qd->dynflags |= QF_DYN_SEQINIT | QF_DYN_SEQADV;
#if QF_DYN_DEBUG
        fprintf(stderr, "init ");
#endif        
    }
    
    /* update MSS */
    if (oct > qd->mss) {
        qd->mss = oct;
#if QF_DYN_DEBUG
        fprintf(stderr, "mss %u ", oct);
#endif
    }
    
    /* update and minimize RTT correction factor if necessary */
    if ((qd->dynflags & QF_DYN_RTTCORR) && (seq >= qd->fan)) {
        qd->dynflags &= ~QF_DYN_RTTCORR;
        qfDynCorrectRTT(qd, ms - qd->fanlms);
#if QF_DYN_DEBUG
        fprintf(stderr, "rttc %u ", qd->rtt_corr);
#endif
    }
   
    /* track sequence numbers in binmap to detect order/loss */
    /* FIXME being reworked */
    if (qfDynHasSeqbin(qd)) {
        if (qfSeqBitsSegmentRtx(&(qd->sb), seq - oct, seq)) {
            qfDynRexmit(qd, seq, ms);
#if QF_DYN_DEBUG
            fprintf(stderr, "rexmit ");
#endif
        }
    }
    
    /* advance sequence number if necessary */
    if (qd->dynflags & QF_DYN_SEQADV || qfSeqCompare(seq, qd->fsn) > 0) {
        qd->dynflags |= QF_DYN_SEQADV;
        if (seq < qd->fsn) qd->wrap_ct++;
        qd->fsn = seq;
#if QF_DYN_DEBUG
        fprintf(stderr, "adv ");
#endif
        
        /* update max inflight */
        if (qd->inflight_max > (qd->fsn - qd->fan)) {
            qd->inflight_max = (qd->fsn - qd->fan);
#if QF_DYN_DEBUG
            fprintf(stderr, "ifmax %u ", qd->inflight_max);
#endif
        }
        
        /* take time sample, if necessary */
        if (qfDynSeqSampleP(qd, ms)) {
            qfSeqRingAddSample(&(qd->sr), seq, ms);
#if QF_DYN_DEBUG
            fprintf(stderr, "rtts ");
#endif
        }
        
    } else {
        qd->dynflags &= ~QF_DYN_SEQADV;
        
        /* update max out of order */
        if ((qd->fsn - seq) > qd->reorder_max) {
            qd->reorder_max = qd->fsn - seq;
#if QF_DYN_DEBUG
            fprintf(stderr, "remax %u ", qd->reorder_max);
#endif
        }
    }
#if QF_DYN_DEBUG
    fprintf(stderr, "\n");
#endif
}

void qfDynAck(qfDyn_t     *qd,
              uint32_t    ack,
              uint32_t    ms)
{
    uint32_t irtt;
    
    /* advance ack number if necessary */
    if (!(qd->dynflags & QF_DYN_ACKINIT) || (qfSeqCompare(ack, qd->fan) > 0)) {
        qd->dynflags |= QF_DYN_ACKINIT;
        qd->fan = ack;
        qd->fanlms = ms;
 
#if QF_DYN_DEBUG
        fprintf(stderr, "qd ts %u ack %10u ", ms, ack);
#endif
        
        /* sample RTT */
        if (qfDynHasSeqring(qd)) {
            irtt = qfSeqRingRTT(&(qd->sr), ack, ms);
            if (irtt) {
                qfDynTrackRTT(qd, irtt);
#if QF_DYN_DEBUG
                fprintf(stderr, "irtt %u ", irtt);
#endif
            }
            qd->dynflags |= QF_DYN_RTTCORR;
        }        
#if QF_DYN_DEBUG
        fprintf(stderr, "\n");
#endif
    }
}

uint64_t qfDynSequenceCount(qfDyn_t *qd, uint8_t flags) {
    uint64_t sc = qd->fsn - qd->isn + (k2e32 * qd->wrap_ct);
    if ((flags & YF_TF_FIN) && sc) sc -= 1;
    return sc;
}

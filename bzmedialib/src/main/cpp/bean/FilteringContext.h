//
/**
 * Created by zhandalin on 2019-06-10 16:43.
 * 说明:
 */
//

#ifndef BZMEDIA_FILTERINGCONTEXT_H
#define BZMEDIA_FILTERINGCONTEXT_H


extern "C" {
#include <libavfilter/avfilter.h>
};

typedef struct FilteringContext {
    AVFilterContext *buffersink_ctx;
    AVFilterContext *buffersrc_ctx;
    AVFilterGraph *filter_graph;
} FilteringContext;

#endif //BZMEDIA_FILTERINGCONTEXT_H

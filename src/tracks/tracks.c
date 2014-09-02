#include "tracks.h"
#include "mod_string.h"
#include "printk.h"
#include "memory.h"
#include "mem_mang.h"

#ifndef RCP_TESTING
#include "memory.h"
static const Tracks g_tracks __attribute__ ((aligned (FLASH_MEMORY_PAGE_SIZE))) __attribute__((section(".tracks\n\t#")));
#else
static Tracks g_tracks = DEFAULT_TRACKS;
#endif

static const Tracks g_defaultTracks = DEFAULT_TRACKS;

static Tracks *g_tracksBuffer = NULL;

void initialize_tracks(){
	if (g_tracks.magicInit != MAGIC_NUMBER_TRACKS_INIT){
		flash_default_tracks();
	}
}

int flash_default_tracks(void){
	pr_info("flashing default tracks...");
	return flash_tracks(&g_defaultTracks, sizeof (g_defaultTracks));
}

int flash_tracks(const Tracks *source, size_t rawSize){
	int result = memory_flash_region(&g_tracks, source, rawSize);
	if (result == 0) pr_info("success\r\n"); else pr_info("failed\r\n");
	return result;
}

const Tracks * get_tracks(){
	return &g_tracks;
}


int add_track(const Track *track, size_t index, int mode){
	int result = TRACK_ADD_RESULT_OK;

	if (index < MAX_TRACK_COUNT){
		if (mode == TRACK_ADD_MODE_IN_PROGRESS || mode == TRACK_ADD_MODE_COMPLETE){
			if (g_tracksBuffer == NULL){
				pr_info("allocating new tracks buffer\r\n");
				g_tracksBuffer = (Tracks *)portMalloc(sizeof(Tracks));
				memcpy(g_tracksBuffer, &g_tracks, sizeof(Tracks));
			}

			if (g_tracksBuffer != NULL){
				Track *trackToAdd = g_tracksBuffer->tracks + index;
				memcpy(trackToAdd, track, sizeof(Track));
				g_tracksBuffer->count = index + 1;

				if (mode == TRACK_ADD_MODE_COMPLETE){
					pr_info("completed updating tracks, flashing: ");
					if (flash_tracks(g_tracksBuffer, sizeof(Tracks)) == 0){
						pr_info("success\r\n");
					}
					else{
						pr_error("error\r\n");
						result = TRACK_ADD_RESULT_FAIL;
					}
					portFree(g_tracksBuffer);
					g_tracksBuffer = NULL;
				}
			}
			else{
				pr_error("could not allocate buffer for tracks\r\n");
				result = TRACK_ADD_RESULT_FAIL;
			}
		}
	}
	else{
		pr_error("invalid track index\r\n");
		result = TRACK_ADD_RESULT_FAIL;
	}
	return result;
}

static int isStage(const Track *t) {
   return t->track_type == TRACK_TYPE_STAGE;
}

int isValidPoint(const GeoPoint *p) {
   return p->latitude && p->longitude;
}

GeoPoint getFinishPoint(const Track *t) {
   return isStage(t) ? t->stage.finish : t->circuit.startFinish;
}

int isFinishPointValid(const Track *t) {
   if (NULL == t)
      return 0;

   const GeoPoint p = getFinishPoint(t);
   return isValidPoint(&p);
}

GeoPoint getStartPoint(const Track *t) {
   return isStage(t) ? t->stage.start : t->circuit.startFinish;
}

int isStartPointValid(const Track *t) {
   if (NULL == t)
      return 0;

   const GeoPoint p = getStartPoint(t);
   return isValidPoint(&p);
}

GeoPoint* getSectorVector(const Track *t) {
   return isStage(t) ? t->stage.sectors : t->circuit.sectors;
}

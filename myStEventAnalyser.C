#include <map>
//#include <TROOT.h>
#include <vector>
#include <iostream>
/*
	struct hitContainer {
		float adc;
		float hitx;
		float hity;
		float hitz;
	} ;
*/
using namespace std;
void myStEventAnalyser(const int numberOfEvents, const char * file, const int eventToSelect = -1, const char * eventsFile = "") {
	cout << file << " to be analyzed" << endl;
	cout << eventToSelect << " to be selected" << endl;
	cout << eventsFile << " will be checked for events to work with" << endl;

//	vector<int> eventVec; // vector for events to display
/*
	FILE * inEvents;
	inEvents = fopen(eventsFile, "r");
	while (!feof (inEvents)) {
		char eventFromFile[100];
		if ( fgets (eventFromFile, 100 , inEvents) == NULL ) break;
		fputs (eventFromFile, stdout);
		cout << eventFromFile << endl;
//		eventVec.push_back(atoi(eventFromFile));
	}
	fclose (inEvents);
*/
//	hitContainer tempHit;

//	map<int, vector<hitContainer> > myHits;

	FILE * outFile;
	FILE * outFileAll;
	FILE * svgOut;
	FILE * outFileT;
	char outFileName[100];
	char outFileAllName[100];
	char svgOutFile[100];
	char outFileNameT[100];

//	const char * file = "st_cosmic_adc_19053068_raw_2000015.event.root";
	gROOT->LoadMacro("$STAR/StRoot/StMuDSTMaker/COMMON/macros/loadSharedLibraries.C");
	loadSharedLibraries();

	gROOT->LoadMacro("bfc.C");
//	TString Chain("StEvent, nodefault, mysql, in");
	TString Chain("StEvent, nodefault, mysql, in, quiet");
	bfc(0, Chain, file); // This will make chain

//	LOOP OVER EVENTS
	int printCounter = 0;
	for (int event = 0; event < numberOfEvents; event++) {
		int iMake = chain->MakeEvent();
		if (iMake % 10 == kStEOF || iMake % 10==kStFatal) {
			cout << "SOMETHING BAD HAPPENED TO THE CHAIN!" << endl;
			break;
		}

		StEvent * pEvent = (StEvent*) chain->GetInputDS("StEvent");
		StSPtrVecTrackNode & myTrackNode = pEvent->trackNodes();

		if (printCounter % 1000 == 0) cout << "Working on event " << printCounter << "(" << event << ")" << " -=- * * * Event: Run "<< pEvent->runId() << " Event No: " << pEvent->id() << " * * * -=-" << endl;
		printCounter++;

		int myNTracks = myTrackNode.size();
/*
//		THIS IS TO SELECT EVENTS WITH LOTS OF TRACKS
		if (myNTracks < 50) {
			if (eventToSelect != -1 && pEvent->id() != eventToSelect) event--;
			continue;
		}
*/
		/*
		if (pEvent->id() > eventToSelect && eventToSelect != -1) {
			cout << "-------------------------------------------------" << endl;
			cout << "EVENT YOU ARE LOOKING FOR IS NOT HERE!\nTHIS POOL STARTS FROM " << pEvent->id() << endl;
			cout << "-------------------------------------------------" << endl;
			return 0;
		}
*/
//		if (eventVec.size != 0 && eventVec.find(pEvent->id()) == null) {
//			event--;
//			continue;
//		}
//		else {	
			if (eventToSelect != -1 && pEvent->id() != eventToSelect) {
				event--;
			       	continue; // event selector condition
			}
//		}

		cout << " -=- * * * Event: Run "<< pEvent->runId() << " Event No: " << pEvent->id() << " * * * -=-" << endl;
		sprintf(outFileName, "StEvent_%d_%d.json", pEvent->runId(),  pEvent->id());
		sprintf(outFileAllName, "AllHits_%d_%d.json", pEvent->runId(),  pEvent->id());
		sprintf(svgOutFile, "hits_%d_%d.svg", pEvent->runId(),  pEvent->id());
		sprintf(outFileNameT, "StTracks_%d_%d.json", pEvent->runId(),  pEvent->id());
		outFile = fopen(outFileName, "w");
		svgOut = fopen(svgOutFile, "w");
		outFileAll = fopen(outFileAllName, "w");
		outFileT = fopen(outFileNameT, "w");

		StTpcHitCollection* TpcHitCollection = pEvent->tpcHitCollection();
		if (!TpcHitCollection) {
			cout << "No TPC Hit Collection in this event, skipping to the next one." << endl;
			continue;
		}
		unsigned int numberOfSectors = TpcHitCollection->numberOfSectors();

//		ADDING HEADER TO THE OUTPUT json FILE
		fprintf(outFile, "{\"EVENT\": {\"R\": %d, \"Evt\": %d, \"B\": 0.5, \"tm\": 1528087733},", pEvent->runId(), pEvent->id());
		fprintf(outFile, "\"META\": {\n\"HITS\": {\"TPC\": {\"type\": \"3D\", \"options\": {\"size\": 5, \"color\": 100255}}},\n\"TRACKS\": {\"type\": \"3D\", \"tracks\": {\"size\":5, \"r_min\": 0, \"r_max\": 2000 }}\n},\n");
		fprintf(outFile, "\"HITS\": {\"TPC\": [\n");

		fprintf(outFileAll, "{\"EVENT\": {\"R\": %d, \"Evt\": %d, \"B\": 0.5, \"tm\": 1528087733},", pEvent->runId(), pEvent->id());
		fprintf(outFileAll, "\"META\": {\n\"HITS\": {\"TPC\": {\"type\": \"3D\", \"options\": {\"size\": 5, \"color\": 100255}}},\n\"TRACKS\": {\"type\": \"3D\", \"tracks\": {\"size\":5, \"r_min\": 0, \"r_max\": 2000 }}\n},\n");
		fprintf(outFileAll, "\"HITS\": {\"TPC\": [\n");

		fprintf(svgOut, "<svg\n\txmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n\t\txmlns:svg=\"http://www.w3.org/2000/svg\"\n\txmlns=\"http://www.w3.org/2000/svg\"\n\tviewBox=\"-323.5 -323.5 647 647\"\n\twidth=\"647\"\n\theight=\"647\"\nt\style=\"background-color: rgb(255, 255, 255);\">");

		fprintf(outFileT, "{\"EVENT\": {	\"R\": %d, 	\"Evt\": %d, 	\"B\": 0.5, 	\"tm\": 1528087733 },\n", pEvent->runId(), pEvent->id());
		fprintf(outFileT, "\t\"META\": {\n\t\t\"TRACKS\": {\"type\": \"3D\", \n\"tracks\": { \n\t\t\t\"size\":5, \"r_min\": 500, \"r_max\": 2000\n\t\t\t}\n\t\t}\n\t},");
		fprintf(outFileT, "\"TRACKS\": { \"tracks\":[\n");

//		BELOW IS MY iTPC 2018 RELATED CHECKS THAT CAN BE REMOVED IF NOT NEEDED ==============================
		for (int sec = 0; sec < 24; sec++) {
			StTpcSectorHitCollection* sectorCollection = TpcHitCollection->sector(sec); //Get sector 20

			unsigned int numberOfPadrows = sectorCollection->numberOfPadrows();
			unsigned long NoHits = 0;
			unsigned long NoBadHits = 0;
			bool doOnce = false;
			for (int row = 0; row < numberOfPadrows; row++) {
				StTpcPadrowHitCollection *rowCollection = sectorCollection->padrow(row);
				if (rowCollection) {
					StSPtrVecTpcHit & hits = rowCollection->hits();
//					checking that there is nothing in padrow > 72
					if (row > 71 && hits.size() != 0) {
						cout << "THERE IS SOMETHING IN UNEXISTING PADROWS!!! (" << row + 1 << ")" << endl;
						return;
					}
					for (int hit = 0; hit < hits.size(); hit++) {
						const StTpcHit * tpcHit = static_cast<const StTpcHit *> (hits[hit]);
						if (tpcHit->flag() == 0) NoHits++;
						NoBadHits++;
//					cout << "tpcHit : " << pEvent->id() << " : " << tpcHit->flag() << ", " <<  tpcHit->pad() << ", " << tpcHit->padrow() << ", " << tpcHit->timeBucket() << ", " << tpcHit->adc() << ", " << tpcHit->position().x() << ", " << tpcHit->position().y() << ", " << tpcHit->position().z() << endl;
//					printf("tpcHit : %d : %d : %d : %d : %.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n", pEvent->id(), tpcHit->flag(), sec, tpcHit->padrow(), tpcHit->pad(), tpcHit->timeBucket(), tpcHit->adc(), tpcHit->position().x(), tpcHit->position().y(), tpcHit->position().z());
//					tempHit.hitx =  tpcHit->position().x();
//					tempHit.hity =  tpcHit->position().y();
//					tempHit.hitz =  tpcHit->position().z();
//					tempHit.adc =  tpcHit->adc();
//					myHits[tpcHit->flag()].push_back(tempHit);
					fprintf(outFileAll, "[%.2f, %.2f, %.2f],\n", tpcHit->position().x(), tpcHit->position().y(), tpcHit->position().z());
//					fprintf(svgOut, "<circle cx=\"%.2f\" cy=\"%.2f\" r=\"1\" stroke=\"none\" stroke-width=\"1\" fill=\"#01879f\" />", tpcHit->position().x(), tpcHit->position().y());
					}
				}
			} // Loop over rows in sector 20
		} // loop over sectors

//		WRITING OUT FILE FOR ALL HITS
//		fprintf(outFile, "{\"EVENT\": {\"R\": %d, \"Evt\": %d, \"B\": 0.5, \"tm\": 1528087733},", pEvent->runId(), pEvent->id());
//		fprintf(outFile, "\"META\": {\n\"HITS\": {\"TPC\": {\"type\": \"3D\", \"options\": {\"size\": 5, \"color\": 100255}}},\n\"TRACKS\": {\"type\": \"3D\", \"tracks\": {\"size\":5, \"r_min\": 0, \"r_max\": 2000 }}\n},\n");
//		fprintf(outFile, "\"HITS\": {\"TPC\": [\n");
/*
		for (map<int, vector<hitContainer> >::iterator mit = myHits.begin(); mit != myHits.end();) {
			for (int vit = 0; vit < mit->second().size(); vit++) {
					fprintf(outFileAll, "[%.2f, %.2f, %.2f]\n", mit->second.hitx, mit->second.hity, mit->second.hitz);
				if (!(vit == mit->second().size() - 1)) fprintf(outFileAll, ",\n");
				esle fprintf(outFileAll, "\n");
			}
			if (++mit == myHits.end()) fprintf(outFileAll, "]\n");
			else fprintf(outFileAll, "],\n");
			
		}
*/

//		if (NoHits < 20) {
//			cout << "Less then 20 hits in sector 20 : "  << NoHits << endl;
//			continue;
//		}

//		=====================================================================================================

		StSPtrVecTrackNode & trackNode = pEvent->trackNodes();
		int nTracks = trackNode.size();
//		if (nTracks < 20) continue; // removing hits from short tracks that are usually not used in the analysis

		StTrackNode * node = 0;
		cout << "Number of tracks : " << nTracks << endl;
		for (int track = 0; track < nTracks; track++) {
			node = trackNode[track];
			if (!node) {cout << "NO NODE" << endl; continue;}

#if 0
//			PRIMARY TRACKS
			StPrimaryTrack* pTrack = static_cast<StPrimaryTrack*>(node->track(primary));
			if (!pTrack) {cout << "NO pTrack" << endl; continue;}
			node = trackNode[track];
			if (!node) {cout << "NO NODE" << endl; continue;}

//			PRIMARY TRACKS
			StPrimaryTrack* pTrack = static_cast<StPrimaryTrack*>(node->track(primary));
			if (!pTrack) {cout << "NO pTrack" << endl; continue;}
			node = trackNode[track];
			if (!node) {cout << "NO NODE" << endl; continue;}

//			PRIMARY TRACKS
			StPrimaryTrack* pTrack = static_cast<StPrimaryTrack*>(node->track(primary));
			if (!pTrack) {cout << "NO pTrack" << endl; continue;}
			if (! pTrack->detectorInfo()) {cout << "=============== detectorInfo is missing" << endl; continue;}
			StPtrVecHit phvec = pTrack->detectorInfo()->hits();

			cout << "Event: Run "<< pEvent->runId() << " Event No: " << pEvent->id() << endl;
			cout << "Track number : " << track << endl;

			for (int hit = 0; hit < phvec.size(); hit++) {
//				if (hvec[hit]->detector() == kTpcId) {
					StTpcHit *tpcHit = static_cast<StTpcHit *> (phvec[hit]);

					if (!tpcHit) continue;
					if (tpcHit->flag() != 0) continue;

					float hitX = tpcHit->position().x();
					float hitY = tpcHit->position().y();
					float hitZ = tpcHit->position().z();
//					cout << tpcHit->pad() << ", " << tpcHit->padrow() << ", " << tpcHit->timeBucket() << ", " <<tpcHit->adc() << ", " << tpcHit->position().x() << endl;
//					if (hitZ > 0) continue;
//					if (tpcHit->padrow() > 40) fprintf(outFile_TPC, "[%.2f, %.2f, %.2f],\n", hitX, hitY, hitZ);
//					else fprintf(outFile_iTPC, "[%.2f, %.2f, %.2f],\n", hitX, hitY, hitZ);
//
					fprintf(outFile, "[%.2f, %.2f, %.2f],\n", hitX, hitY, hitZ);


//					cout << "[" << hitX << ", " << hitY << ", " << hitZ << "]," << endl;
//					fprintf(outFile, "%d, %d, %d, %d, %f, %f, %f, %f\n", track, tpcHit->pad(), tpcHit->padrow(), tpcHit->timeBucket(), tpcHit->adc(), hitX, hitY, hitZ);
//				}
			}
#endif

#if 1
//			GLOBAL TRACKS
			StGlobalTrack* gTrack = static_cast<StGlobalTrack*>(node->track(global));
			if (!gTrack) {cout << "NO gTrack" << endl; continue;}
			if (! gTrack->detectorInfo()) {cout << "=============== detectorInfo is missing" << endl; continue;}
			StPtrVecHit ghvec = gTrack->detectorInfo()->hits();
			StTrackGeometry * gTrackParams = gTrack->geometry();
			if (!gTrackParams) continue;

//			fprintf(outFileT, "{\"pt\": %.3f,\"xyz\":[%.3f, %.3f, %.3f], \"pxyz\":[%.3f, %.3f, %.3f], \"q\": %d,\"l\": %.3f,\"nh\":20},\n", gTrackParams->pt(), gTrackParams->momentum().x(), gTrackParams->momentum().y(), gTrackParams->momentum().z(), gTrackParams->origin().x(), gTrackParams->origin().y(), gTrackParams->origin().z(), gTrackParams->charge(), gTrack->length());
//			fprintf(outFileT, "{\"pt\": %.3f,\"xyz\":[%.3f, %.3f, %.3f], \"pxyz\":[%.3f, %.3f, %.3f], \"q\": %d,\"l\": %.3f,\"nh\":20},\n", gTrackParams->pt(),  gTrackParams->origin().x(), gTrackParams->origin().y(), gTrackParams->origin().z(), gTrackParams->momentum().x(), gTrackParams->momentum().y(), gTrackParams->momentum().z(), gTrackParams->charge(), gTrack->length());
			fprintf(outFileT, "{\"pt\": %.3f,\"xyz\":[%.3f, %.3f, %.3f], \"pxyz\":[%.3f, %.3f, %.3f], \"q\": %d,\"l\": %.3f,\"nh\":20},\n", gTrackParams->momentum().perp(),  gTrackParams->origin().x(), gTrackParams->origin().y(), gTrackParams->origin().z(), gTrackParams->momentum().x(), gTrackParams->momentum().y(), gTrackParams->momentum().z(), gTrackParams->charge(), gTrack->length());

			int color = track*10;

			for (int hit = 0; hit < ghvec.size(); hit++) {
//				if (hvec[hit]->detector() == kTpcId) {
					StTpcHit *tpcHit = static_cast<StTpcHit *> (ghvec[hit]);
					fprintf(svgOut, "<circle cx=\"%.2f\" cy=\"%.2f\" r=\"1\" stroke=\"none\" stroke-width=\"1\" style=\"fill:rgb(150, %d, %d)\" />\n", tpcHit->position().x(), tpcHit->position().y(), 100 + color, 200 - color);
                                       if (!tpcHit) continue;
                                       if (tpcHit->flag() != 0) continue;
                                       float hitX = tpcHit->position().x();
                                       float hitY = tpcHit->position().y();
                                       float hitZ = tpcHit->position().z();
//                                     cout << tpcHit->pad() << ", " << tpcHit->padrow() << ", " << tpcHit->timeBucket() << ", " <<tpcHit->adc() << ", " << tpcHit->position().x() << endl;
//                                     if (hitZ > 0) continue;
//                                     if (tpcHit->padrow() > 40) fprintf(outFile_TPC, "[%.2f, %.2f, %.2f],\n", hitX, hitY, hitZ);
//                                     else fprintf(outFile_iTPC, "[%.2f, %.2f, %.2f],\n", hitX, hitY, hitZ);
                                       fprintf(outFile, "[%.2f, %.2f, %.2f],\n", hitX, hitY, hitZ);
//                                     cout << "[" << hitX << ", " << hitY << ", " << hitZ << "]," << endl;

//					if (!tpcHit || tpcHit->sector() != 20) continue;
//					fprintf(outFile, "%d, %d, %d, %d, %f, %f, %f, %f\n", track, tpcHit->pad(), tpcHit->padrow(), tpcHit->timeBucket(), tpcHit->adc(), hitX, hitY, hitZ);
//				}
			}
#endif		

		}
		fprintf(outFile, "[0,0,0]]\n}\n}");
		fprintf(outFileAll, "[0,0,0]]\n}\n}");
		fprintf(svgOut, "</svg>");
		fprintf(outFileT, "{}\n]\n}\n}");
		fclose(outFile);
		fclose(outFileT);
	} // Event Loop
}

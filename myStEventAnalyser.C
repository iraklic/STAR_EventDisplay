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
void myStEventAnalyser(int numberOfEvents, const char * file, int eventToSelect = -1, const char * eventsFile = "") {
	cout << file << " to be analyzed" << endl;
	cout << eventToSelect << " to be selected" << endl;

	vector<int> eventVec; // vector for events to display

	FILE * inEvents;
	if (eventsFile != "") {
		cout << eventsFile << " will be checked for events to work with" << endl;
		inEvents = fopen(eventsFile, "r");
		while (!feof (inEvents)) {
			char eventFromFile[100];
			if ( fgets (eventFromFile, 100 , inEvents) == NULL ) break;
			eventVec.push_back(atoi(eventFromFile));
		}
		fclose (inEvents);
		cout << "There is a whole file from which I'll pick interesting events!" << endl;
		numberOfEvents = eventVec.size();
	}
	

//	hitContainer tempHit;

//	map<int, vector<hitContainer> > myHits;

	FILE * dataOut;
	FILE * hitMap;
	FILE * hitsOnTracks;

	char dataOutName[100];
	char hitMapName[100];
	char hitsOnTracksName[100];

//	const char * file = "st_cosmic_adc_19053068_raw_2000015.event.root";
	gROOT->LoadMacro("$STAR/StRoot/StMuDSTMaker/COMMON/macros/loadSharedLibraries.C");
	loadSharedLibraries();

	gROOT->LoadMacro("bfc.C");
	TString Chain("StEvent, nodefault, mysql, in");
//	TString Chain("StEvent, nodefault, mysql, in, quiet");
	bfc(0, Chain, file); // This will make chain

//	PREPARE OUTPUT FILES
	const char * fBaseName = gSystem->BaseName(file);
	TString fBaseN(fBaseName);
	fBaseN.ReplaceAll(".event.root", "");

	sprintf(hitsOnTracksName, "%s_hitsOnTracks.csv", fBaseN.Data());
	hitsOnTracks = fopen(hitsOnTracksName, "w");
	fprintf(hitsOnTracks, "eventId, track, flag, sector, padrow, padMin, padMax, adc, x, y, z, r, g, b\n");

	sprintf(dataOutName, "%s_tracks.csv", fBaseN.Data());
	dataOut = fopen(dataOutName, "w");
	fprintf(dataOut, "eventId, track, xi, yi, zi, xf, yf, zf, px, py, pz, r, g, b, c, curv, pt, length\n");

	sprintf(hitMapName, "%s_hitMap.csv", fBaseN.Data());
	hitMap = fopen(hitMapName, "w");
	fprintf(hitMap, "eventId, sector, row, padMin, padMax, adc, x, y, z\n");

//	LOOP OVER EVENTS
	int printCounter = 0;
	for (int event = 0; event < numberOfEvents; event++) {
		int iMake = chain->MakeEvent();
		if (iMake % 10 == kStEOF || iMake % 10==kStFatal) {
			cout << "SOMETHING BAD HAPPENED TO THE CHAIN!" << endl;
			break;
		}

		StEvent * pEvent = (StEvent*) chain->GetInputDS("StEvent");
		if (!pEvent) {
			cout << "    - - - - - - - !!! - - - - - - - There is no StEvent?!" << endl;
			continue;
		}
		StSPtrVecTrackNode & myTrackNode = pEvent->trackNodes();

		if (printCounter % 1000 == 0) cout << "Working on event " << printCounter << "(" << event << ")" << " -=- * * * Event: Run "<< pEvent->runId() << " Event No: " << pEvent->id() << " * * * -=-" << endl;
		printCounter++;

		int myNTracks = myTrackNode.size();

//		THIS IS TO SELECT EVENTS WITH SOME PRESET CUTS
#if 0
		StPrimaryVertex * pv = pEvent->primaryVertex();
//		double pvz = pv->position().z();
//		if (fabs(pvz) > 5) {
//			event--;
//			continue;
//		}
		if (myNTracks < 500) {
			event--;
			continue;
		}
# endif


		if (eventsFile != "") eventToSelect = eventVec.at(event);

		if (eventToSelect != -1 && pEvent->id() != eventToSelect) {
			cout << eventToSelect << " ---------------------------- " << endl;
			event--;
			if (eventToSelect < pEvent->id()) event++;
		       	continue; // event selector condition
		}

		StTpcHitCollection* TpcHitCollection = pEvent->tpcHitCollection();
		if (!TpcHitCollection) {
			cout << "No TPC Hit Collection in this event, skipping to the next one." << endl;
			continue;
		}

		cout << " -=- * * * Event: Run "<< pEvent->runId() << " Event No: " << pEvent->id() << " * * * -=-" << endl;

//		All hits and sector loop below ==============================
		for (int sec = 0; sec < 24; sec++) {
			StTpcSectorHitCollection* sectorCollection = TpcHitCollection->sector(sec);

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

						fprintf(hitMap, "%d, %d, %d, %d, %d, %f, %.2f, %.2f, %.2f\n", pEvent->id(), sec, row, tpcHit->minPad(), tpcHit->maxPad(), tpcHit->adc(), tpcHit->position().x(), tpcHit->position().y(), tpcHit->position().z());
					}
				}
			} // Loop over rows in sector 20
		} // loop over sectors


//		BTOW HITS ==========================================================================================
//		StEmcCollection * emc = pEvent->emcCollection();
//		for(int btowHit = 0; btowHit < 4800; btowHit++) {
//			StPicoBTowHit *tower = static_cast<StPicoBTowHit*>(mPicoDst->btowHit(j));
//			double TowerE = tower->energy();
//		}

//		====================================================================================================


		StSPtrVecTrackNode & trackNode = pEvent->trackNodes();
		int nTracks = trackNode.size();
//		if (nTracks < 100) continue; // removing hits from short tracks that are usually not used in the analysis

		if (!nTracks) continue;

		StTrackNode * node = 0;
		cout << "Number of tracks : " << nTracks << endl;

		for (int trackNumber = 0; trackNumber < nTracks; trackNumber++) {
			node = trackNode[trackNumber];
			if (!node) {
				cout << "NO NODE" << endl;
				continue;
			}

			StGlobalTrack* track = static_cast<StGlobalTrack*>(node->track(global));
//			StPrimaryTrack* track = static_cast<StPrimaryTrack*>(node->track(primary));
			if (!track) continue;
			if (! track->detectorInfo()) {cout << "=============== detectorInfo is missing" << endl; continue;}

			StPtrVecHit phvec = track->detectorInfo()->hits();
			StTrackGeometry * trackParams = track->geometry();
			StTrackGeometry * o_trackParams = track->outerGeometry();
			if (!trackParams) {cout << "-=- No track params for primary track -=-" << endl; continue;}

//			CUTS ------------------------------------------
#if 0
//			if (trackParams->momentum().pseudoRapidity() > 0.5) continue;
			double phi_track = trackParams->momentum().phi();
			double eta_track = trackParams->momentum().pseudoRapidity();
			if (!(sqrt((3.73224 - phi_track)*(3.73224 - phi_track) + (0.358856 - eta_track)*(0.358856 - eta_track)) < 0.4 ||
				sqrt((0.596783 - phi_track)*(0.596783 - phi_track) + (-0.17974 - eta_track)*(-0.17974 - eta_track)) < 0.4)) continue;
//			CUTS ------------------------------------------
# endif

//			if (track->idTruth() != 8) continue;
//			cout << track->idTruth() << endl;

			int up = -1; // 1 is vertically up y > 0 and -1 is vertically down y < 0

//			BELOW IS THE COLOR SETUP FOR THE HITS ON THE TRACK ACCORDING TO THE STAR COLOR SCHEME
			double int myR, myG, myB;
			double trackP = trackParams->momentum().perp();
			double maxP = 4.5;
			double colval = trackP < maxP ? trackP / maxP : 1;
			double colvaltimes4 = colval * 4.0;

			if ( colval < 0.25 ) {
				myG = colvaltimes4;
				myB = myG;
				myB += 1.0 - colvaltimes4;
			}
			else if ( colval < 0.5 ) {
				myG = 1.0 - ( colvaltimes4 - 1.0 );
				myB = myG;
				myG += colvaltimes4 - 1.0;
			}
			else if ( colval < 0.75 ) {
				myR = colvaltimes4 - 2.0;
				myG = myR;
				myG += 1.0 - ( colvaltimes4 - 2.0 );
			}
			else {
				myR = 1.0 - ( colvaltimes4 - 3.0 );
				myG = myR;
				myR += colvaltimes4 - 3.0;
			}
			myR *= 255;
			myG *= 255;
			myB *= 255;

			int myCharge = trackParams->charge() == 1 ? 1 : 0;

//			if (phvec.size() < 20) continue; // SELECTION FOR NUMBER OF HITS ON THE TRACK
			bool doIneedThisTrack = true;
//			cout << "Number of hits on this track : " << phvec.size() << endl;
			for (int hit = 0; hit < phvec.size(); hit++) {
//				if (hvec[hit]->detector() == kTpcId) {
					StTpcHit *tpcHit = static_cast<StTpcHit *> (phvec[hit]);

					if (!tpcHit) continue;
//					if (tpcHit->flag() != 0) continue;
//					if (tpcHit->flag() > 1) continue;

					float hitX = tpcHit->position().x();
					float hitY = tpcHit->position().y();
					float hitZ = tpcHit->position().z();
					
					fprintf(hitsOnTracks, "%d, %d, %d, %d, %d, %d, %d, %d, %.2f, %.2f, %.2f, %d, %d, %d\n", pEvent->id(), trackNumber, tpcHit->flag(), tpcHit->sector(), tpcHit->padrow(), tpcHit->minPad(), tpcHit->maxPad(), tpcHit->adc(), hitX, hitY, hitZ, myR, myG, myB);
			}

			double length = TMath::Sqrt(o_trackParams->origin().x() * o_trackParams->origin().x() + o_trackParams->origin().y() * o_trackParams->origin().y());
			double segmentLength = trackParams->momentum().perp() * 40/2;
			double segmentLength1 = (o_trackParams->origin().z() + 410) * segmentLength/410;

			fprintf(dataOut, "%d, %d, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %d, %d, %d, %d, %.2f, %.2f, %.2f\n", pEvent->id(), trackNumber,
					trackParams->origin().x(), trackParams->origin().y(), trackParams->origin().z(),
					o_trackParams->origin().x(), o_trackParams->origin().y(), o_trackParams->origin().z(),
					trackParams->momentum().x(), trackParams->momentum().y(), trackParams->momentum().z(),
					myR, myG,myB, 
					myCharge, 1/trackParams->curvature(), trackParams->momentum().perp(), track->length());
		}
	} // Event Loop
	fclose(hitMap);
	fclose(dataOut);
	fclose(hitsOnTracks);
}

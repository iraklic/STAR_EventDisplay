void myStEventAnalyser(const char * file, const int eventToSelect = -1) {
	cout << file << " to be analyzed" << endl;
	cout << eventToSelect << " to be selected" << endl;

	FILE * outFile;
	char outFileName[100];

//	const char * file = "st_cosmic_adc_19053068_raw_2000015.event.root";
	gROOT->LoadMacro("$STAR/StRoot/StMuDSTMaker/COMMON/macros/loadSharedLibraries.C");
	loadSharedLibraries();

	gROOT->LoadMacro("bfc.C");
//	TString Chain("StEvent, nodefault, mysql, in");
	TString Chain("StEvent, nodefault, mysql, in, quiet");
	bfc(0, Chain, file); // This will make chain

//	LOOP OVER EVENTS
	for (int event = 0; event < 200000; event++) {
		int iMake = chain->MakeEvent();
		if (iMake % 10 == kStEOF || iMake % 10==kStFatal) break;

		StEvent * pEvent = (StEvent*) chain->GetInputDS("StEvent");
		if (event % 1000 == 0) cout << "Working on event " << event << " -=- * * * Event: Run "<< pEvent->runId() << " Event No: " << pEvent->id() << " * * * -=-" << endl;

		if (eventToSelect != -1 && pEvent->id() != eventToSelect) continue; // event selector condition

		cout << " -=- * * * Event: Run "<< pEvent->runId() << " Event No: " << pEvent->id() << " * * * -=-" << endl;
		sprintf(outFileName, "StEvent_%d_%d.json", pEvent->runId(),  pEvent->id());
		outFile = fopen(outFileName, "w");

		StTpcHitCollection* TpcHitCollection = pEvent->tpcHitCollection();
		if (!TpcHitCollection) {
			cout << "No TPC Hit Collection" << endl;
			return;
		}
		unsigned int numberOfSectors = TpcHitCollection->numberOfSectors();

//		ADDING HEADER TO THE OUTPUT json FILE
		fprintf(outFile, "{\"EVENT\": {\"R\": %d, \"Evt\": %d, \"B\": 0.5, \"tm\": 1528087733},", pEvent->runId(), pEvent->id());
		fprintf(outFile, "\"META\": {\n\"HITS\": {\"TPC\": {\"type\": \"3D\", \"options\": {\"size\": 5, \"color\": 100255}}},\n\"TRACKS\": {\"type\": \"3D\", \"tracks\": {\"size\":5, \"r_min\": 0, \"r_max\": 2000 }}\n},\n");
		fprintf(outFile, "\"HITS\": {\"TPC\": [\n");
		
//		BELOW IS MY iTPC 2018 RELATED CHECKS THAT CAN BE REMOVED IF NOT NEEDED ==============================
		StTpcSectorHitCollection* sectorCollection = TpcHitCollection->sector(19); //Get sector 20

		unsigned int numberOfPadrows = sectorCollection->numberOfPadrows();
		unsigned long NoHits = 0;
		unsigned long NoBadHits = 0;
		bool doOnce = false;
		for (int row = 0; row < numberOfPadrows; row++) {
			StTpcPadrowHitCollection *rowCollection = sectorCollection->padrow(row);
			if (rowCollection) {
				StSPtrVecTpcHit & hits = rowCollection->hits();
//				checking that there is nothing in padrow > 72
				if (row > 71 && hits.size() != 0) {
					cout << "THERE IS SOMETHING IN UNEXISTING PADROWS!!! (" << row + 1 << ")" << endl;
					return;
				}
				for (int hit = 0; hit < hits.size(); hit++) {
					const StTpcHit * tpcHit = static_cast<const StTpcHit *> (hits[hit]);
					if (tpcHit->flag() == 0) NoHits++;
					NoBadHits++;
				}
			}
		} // Loop over rows in sector 20

//		if (NoHits < 20) {
//			cout << "Less then 20 hits in sector 20 : "  << NoHits << endl;
//			continue;
//		}

//		=====================================================================================================

		StSPtrVecTrackNode & trackNode = pEvent->trackNodes();
		int nTracks = trackNode.size();
//		if (nTracks < 20) continue; // removing hits from short tracks that are usually not used in the analysis

		StTrackNode * node = 0;
		for (int track = 0; track < nTracks; track++) {
			node = trackNode[track];
			if (!node) continue;

#if 0
//			PRIMARY TRACKS
			StPrimaryTrack* pTrack = static_cast<StPrimaryTrack*>(node->track(primary));
			if (!pTrack) continue;
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
			if (!gTrack) continue;
			if (! gTrack->detectorInfo()) {cout << "=============== detectorInfo is missing" << endl; continue;}
			StPtrVecHit ghvec = gTrack->detectorInfo()->hits();
			cout << " - - - -- - - - - " << ghvec.size() << endl;
			for (int hit = 0; hit < ghvec.size(); hit++) {
//				if (hvec[hit]->detector() == kTpcId) {
					StTpcHit *tpcHit = static_cast<StTpcHit *> (ghvec[hit]);

//					if (!tpcHit || tpcHit->sector() != 20) continue;
					if (!tpcHit) continue;
					if (tpcHit->flag() != 0) continue;

					float hitX = tpcHit->position().x();
					float hitY = tpcHit->position().y();
					float hitZ = tpcHit->position().z();
//					cout << tpcHit->pad() << ", " << tpcHit->padrow() << ", " << tpcHit->timeBucket() << ", " <<tpcHit->adc() << ", " << tpcHit->position().x() << endl;
//					if (hitZ > 0) continue;
//					if (tpcHit->padrow() > 40) fprintf(outFile_TPC, "[%.2f, %.2f, %.2f],\n", hitX, hitY, hitZ);
//					else fprintf(outFile_iTPC, "[%.2f, %.2f, %.2f],\n", hitX, hitY, hitZ);
					fprintf(outFile, "[%.2f, %.2f, %.2f],\n", hitX, hitY, hitZ);
//					cout << "[" << hitX << ", " << hitY << ", " << hitZ << "]," << endl;
//					fprintf(outFile, "%d, %d, %d, %d, %f, %f, %f, %f\n", track, tpcHit->pad(), tpcHit->padrow(), tpcHit->timeBucket(), tpcHit->adc(), hitX, hitY, hitZ);
//				}
			}
#endif		

		}
		fprintf(outFile, "[0,0,0]]\n}\n}");
		fclose(outFile);
	} // Event Loop
}

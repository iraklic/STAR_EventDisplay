void myStEventAnalyser.C() {
	FILE * outFile;
	outFile = fopen("out.csv", "w");
	const char * file = "st_cosmic_adc_19053068_raw_2000015.event.root";
	gROOT->LoadMacro("$STAR/StRoot/StMuDSTMaker/COMMON/macros/loadSharedLibraries.C");
	loadSharedLibraries();

	gROOT->LoadMacro("bfc.C");
	TString Chain("StEvent, nodefault, mysql, in");
	bfc(0, Chain, file); // This will make chain

	for (int event = 0; event < 10; event++) {
		int iMake = chain->MakeEvent();
		if (iMake % 10 == kStEOF || iMake % 10==kStFatal) break;

		StEvent * pEvent = (StEvent*) chain->GetInputDS("StEvent");
		cout << "Event: Run "<< pEvent->runId() << " Event No: " << pEvent->id() << endl;

		StTpcHitCollection* TpcHitCollection = pEvent->tpcHitCollection();
		if (!TpcHitCollection) {
			cout << "No TPC Hit Collection" << endl;
			return;
		}
		unsigned int numberOfSectors = TpcHitCollection->numberOfSectors();
		StTpcSectorHitCollection* sectorCollection = TpcHitCollection->sector(19); //Get sector 20

		unsigned int numberOfPadrows = sectorCollection->numberOfPadrows();
		unsigned long NoHits = 0;
		unsigned long NoBadHits = 0;
		for (int row = 0; row < numberOfPadrows; row++) {
			StTpcPadrowHitCollection *rowCollection = sectorCollection->padrow(row);
			if (rowCollection) {
				StSPtrVecTpcHit & hits = rowCollection->hits();
//				checking that there is nothing in padrow > 72
				if (row > 71 && hits.size() != 0) {
					cout << "THRE IS SOMETHING IS UNEXISTING PADROWS!!! (" << row + 1 << ")" << endl;
					return;
				}
				for (int hit = 0; hit < hits.size(); hit++) {
					const StTpcHit * tpcHit = static_cast<const StTpcHit *> (hits[hit]);
					if (tpcHit->flag() == 0) NoHits++;
					NoBadHits++;
				}
			}
		} // Loop over rows in sector 20

		cout << "\t NoHits = " << NoHits << endl;
		cout << "\t NoBadHits = " << NoBadHits << endl;
		if (NoHits < 20) continue;

		StSPtrVecTrackNode & trackNode = pEvent->trackNodes();
		int nTracks = trackNode.size();
		StTrackNode * node = 0;
		for (int track = 0; track < nTracks; track++) {
			node = trackNode[track];
			if (!node) continue;
			StGlobalTrack* gTrack = static_cast<StGlobalTrack*>(node->track(global));
			if (!gTrack) continue;
//			cout << *gTrack << endl;
			if (! gTrack->detectorInfo()) {cout << "=============== detectorInfo is missing" << endl; continue;}
			StPtrVecHit hvec = gTrack->detectorInfo()->hits();
			for (int hit = 0; hit < hvec.size(); hit++) {
				if (hvec[hit]->detector() == kTpcId) {
					StTpcHit *tpcHit = static_cast<StTpcHit *> (hvec[hit]);
					if (!tpcHit || tpcHit->sector() != 20) continue;
					cout << tpcHit->pad() << ", " << tpcHit->padrow() << ", " << tpcHit->timeBucket() << ", " <<tpcHit->adc() << endl;
					fprintf(outFile, "%d, %d, %d, %d, %d, %f\n", event, track, tpcHit->pad(), tpcHit->padrow(), tpcHit->timeBucket(), tpcHit->adc());
				}
			}
		}
	} // Event Loop
	fclose(outFile);
}

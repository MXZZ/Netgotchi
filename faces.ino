
void drawnetgotchiFace(int state) {
  displaySetCursor(30 + moveX, 30);
  if (evilTwinDetected) {
    netgotchiCurrentFace = netgotchiFaceEvilTwin;
  } else if (honeypotTriggered) {
    if (state == 0) netgotchiCurrentFace = netgotchiFaceSad;
    if (state == 1) netgotchiCurrentFace = netgotchiFaceSad2;
    if (state == 2) netgotchiCurrentFace = netgotchiFaceSuspicious;
    if (state == 3) netgotchiCurrentFace = netgotchiFaceSuspicious2;
    if (state == 4) netgotchiCurrentFace = netgotchiFaceHit;
    if (state == 5) netgotchiCurrentFace = netgotchiFaceHit2;
  } else {
    if (state == 0) netgotchiCurrentFace = netgotchiFace;
    if (state == 1) netgotchiCurrentFace = netgotchiFace2;
    if (state == 2) netgotchiCurrentFace = netgotchiFaceBlink;
    if (state == 3) netgotchiCurrentFace = netgotchiFaceSleep;
    if (state == 4) netgotchiCurrentFace = netgotchiFaceSurprised;
    if (state == 5) netgotchiCurrentFace = netgotchiFaceHappy;
  }
  displayPrintln(netgotchiCurrentFace);
}

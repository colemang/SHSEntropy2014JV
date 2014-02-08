#ifndef VISION_H_
#define VISION_H_


class Vision
{
	struct Scores {
		double rectangularity;
		double aspectRatioVertical;
		double aspectRatioHorizontal;
	};
	
	struct TargetReport {
		int verticalIndex;
		int horizontalIndex;
		bool Hot;
		double totalScore;
		double leftScore;
		double rightScore;
		double tapeWidthScore;
		double verticalScore;
	};
	
public:

	double TakeDistancePicture(DriverStationLCD* ds, int bottomH, int topH, int bottomS, int topS, int bottomV, int topV);

protected:
	double computeDistance (BinaryImage *image, ParticleAnalysisReport *report);
	double scoreAspectRatio(BinaryImage *image, ParticleAnalysisReport *report, bool vertical);
	bool scoreCompare(Scores scores, bool vertical);
	double scoreRectangularity(ParticleAnalysisReport *report);
	double ratioToScore(double ratio);
	bool hotOrNot(TargetReport target);
};
	
	


#endif

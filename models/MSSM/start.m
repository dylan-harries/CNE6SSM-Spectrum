$flexiblesusyOutputDir   = "models/CMSSM";
$flexiblesusyConfigDir   = FileNameJoin[{Directory[], "config"}];
$flexiblesusyMetaDir     = FileNameJoin[{Directory[], "meta"}];
$flexiblesusyTemplateDir = FileNameJoin[{Directory[], "templates"}];
AppendTo[$Path, $flexiblesusyMetaDir];

Needs["SARAH`"];
Needs["FlexibleSUSY`", FileNameJoin[{$flexiblesusyMetaDir, "FlexibleSUSY.m"}]];

workingDirectory = Directory[];
SARAH`SARAH[OutputDirectory] = FileNameJoin[{workingDirectory, "Output"}];
SARAH`SARAH[InputDirectories] = {
    FileNameJoin[{workingDirectory, "sarah"}],
    ToFileName[{$sarahDir, "Models"}]
};

Print["Current working directory: ", workingDirectory];
Print["SARAH output directory: ", SARAH`SARAH[OutputDirectory]];
Print["Starting model: ", ToString[HoldForm[Start["MSSM"]]]];

Start["MSSM"];

MakeFlexibleSUSY[InputFile -> FileNameJoin[{"models/CMSSM","FlexibleSUSY.m"}],
                 DebugOutput -> False];

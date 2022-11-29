<#
.Synopsis
    Invokes a command and imports its environment variables.
    Author: Roman Kuzmin (inspired by Lee Holmes's Invoke-CmdScript.ps1)

.Description
    It invokes the specified command or batch file with arguments and imports
    its result environment variables to the current PowerShell session.

    Command output is discarded by default, use the switch Output to enable it.
    You may check for $LASTEXITCODE unless the switch Force is specified.

.Parameter Command
        Specifies the entire command including the batch file and parameters.
        This string is passed in `cmd /c` as it is. Mind quotes for paths and
        arguments with spaces and special characters. Do not use redirection.

.Parameter File
        Specifies the batch file path.

.Parameter Arguments
        With File, specifies its arguments. Arguments with spaces are quoted.
        In the batch file, you may unquote them as %~1, %~2, etc.
        Other special characters are not treated.

.Parameter Output
        Tells to collect and return the command output.

.Parameter Force
        Tells to import variables even if the command exit code is not 0.

.Inputs
    None. Use the script parameters.

.Outputs
    With Output, the command output.

.Example
    >
    # Invoke vsvars32 and import variables even if exit code is not 0
    Invoke-Environment '"%VS100COMNTOOLS%\vsvars32.bat"' -Force

    # Invoke vsvars32 as file and get its output
    Invoke-Environment -File $env:VS100COMNTOOLS\vsvars32.bat -Output

.Link
    https://github.com/nightroman/PowerShelf
#>

[CmdletBinding(DefaultParameterSetName='Command')]
param(
    [Parameter(Mandatory=1, Position=0, ParameterSetName='Command')]
    [string]$Command
    ,
    [Parameter(Mandatory=1, ParameterSetName='File')]
    [string]$File
    ,
    [Parameter(ParameterSetName='File')]
    [string[]]$Arguments
    ,
    [switch]$Output
    ,
    [switch]$Force
)

$stream = if ($Output) {($temp = [IO.Path]::GetTempFileName())} else {'nul'}
$operator = if ($Force) {'&'} else {'&&'}

if ($File) {
    $Command = "`"$File`""
    if ($Arguments) {
        foreach($_ in $Arguments) {
            if ($_.Contains(' ')) {
                $_ = "`"$_`""
            }
            $Command += " $_"
        }
    }
}

foreach($_ in cmd.exe /c " $Command > `"$stream`" 2>&1 $operator SET") {
    if ($_ -match '^([^=]+)=(.*)') {
        [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2])
    }
}

if ($Output) {
    Get-Content -LiteralPath $temp
    Remove-Item -LiteralPath $temp
}

/* 
 * Various exceptions (hopefully not) thrown by mmpbsa classes.
 *
 * Created by David Coss <David.Coss@stjude.org> 2010
 */
#include <stdexcept>

#ifndef MMPBSA_EXCEPTIONS_H
#define	MMPBSA_EXCEPTIONS_H

namespace mmpbsa{

//MMPBSAErrorTypes could be used as return values. Therefore, zero is not used.
enum MMPBSAErrorTypes {UNKNOWN_ERROR = 1, /*<Avoid, as this is vague.*/
    FILE_READ_ERROR,/*<IO problem reading prmtop file.*/
    BROKEN_PRMTOP_FILE,/*<prmtop file is improperly formatted or missing data.*/
    BROKEN_TRAJECTORY_FILE,/*<trajectory file is improperly formatted or missing data.*/
    INVALID_PRMTOP_DATA,/*<Data which was loaded into an array is incorrect based on what is expected.*/
    DATA_FORMAT_ERROR,/*<Use this error, when data *within* the program no long matches what it should due to formatting problems.*/
    INVALID_ARRAY_SIZE,
    UNEXPECTED_EOF,
    COMMAND_LINE_ERROR,/*<Program supplied an invalide argument in the command line.*/
    BAD_XML_TAG
};

class MMPBSAException : public std::runtime_error
{
    public:
        /**
         * Very General MMPBSAException. Use more specific exception if one is
         * available.
         *
         * @param error
         */
    MMPBSAException( const std::string& error) : runtime_error(error) {errorType = UNKNOWN_ERROR;}
    /**
     * Creates an exception, with a specified error type.
     *
     * @param error
     * @param errorType
     */
    MMPBSAException(const std::string& error, const MMPBSAErrorTypes& errorType) : runtime_error(error){this->errorType = errorType;}
    
    /**
     * Returns the error type, corresponding to the error types listed below.
     * These should be returned if the exception is caught and the program
     * dies gracefully.
     *
     * @return int error type.
     */
    MMPBSAErrorTypes getErrType(){return errorType;}


    virtual const char* identifier(){return "General MMPBSA Error";}

private:
    MMPBSAErrorTypes errorType;

};


class SanderIOException : public MMPBSAException {
public:
    SanderIOException(const std::string& error) : MMPBSAException( error){}
    
    SanderIOException(const std::string& error, const MMPBSAErrorTypes& errorType)
        : MMPBSAException(error,errorType){}

    const char* identifier(){return "SanderIO Error";}
};

};//end namespace mmpbsa

#endif	/* MMPBSA_EXCEPTIONS_H */

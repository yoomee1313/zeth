// DISCLAIMER:
// Content taken and adapted from:
// https://github.com/HarryR/ethsnarks/blob/master/src/gadgets/mimc.hpp

#ifndef __ZETH_CIRCUITS_MIMC_PERMUTATION_TCC__
#define __ZETH_CIRCUITS_MIMC_PERMUTATION_TCC__

#include "libzeth/circuits/mimc/mimc_permutation.hpp"

namespace libzeth
{

template<typename FieldT, size_t Exponent, size_t NumRounds>
std::vector<FieldT>
    MiMC_permutation_gadget<FieldT, Exponent, NumRounds>::round_constants;

template<typename FieldT, size_t Exponent, size_t NumRounds>
bool MiMC_permutation_gadget<FieldT, Exponent, NumRounds>::
    round_constants_initialized = false;

template<typename FieldT, size_t Exponent, size_t NumRounds>
MiMC_permutation_gadget<FieldT, Exponent, NumRounds>::MiMC_permutation_gadget(
    libsnark::protoboard<FieldT> &pb,
    const libsnark::pb_linear_combination<FieldT> &msg,
    const libsnark::pb_linear_combination<FieldT> &key,
    const libsnark::pb_variable<FieldT> &result,
    const libsnark::pb_linear_combination<FieldT> &add_to_result,
    const bool add_to_result_is_valid,
    const std::string &annotation_prefix)
    : libsnark::gadget<FieldT>(pb, annotation_prefix)
{
    // Ensure round constants are initialized.
    setup_sha3_constants();

    // Initialize the round gadgets
    round_gadgets.reserve(NumRounds);

    // First round uses round_msg as an input.
    round_results[0].allocate(
        this->pb, FMT(this->annotation_prefix, " round_result[0]"));
    round_gadgets.emplace_back(
        this->pb,
        msg,
        key,
        round_constants[0],
        round_results[0],
        FMT(this->annotation_prefix, " round[0]"));

    // Intermediate rounds use the output of the previous round and output to
    // an intermediate variable (allocated here)
    for (size_t i = 1; i < NumRounds - 1; i++) {
        // Allocate intermediate round result.
        round_results[i].allocate(
            this->pb, FMT(this->annotation_prefix, " round_result[%zu]", i));

        // Initialize the current round gadget into the vector of round gadgets
        // vector, picking the correct round constant.
        round_gadgets.emplace_back(
            this->pb,
            round_results[i - 1],
            key,
            round_constants[i],
            round_results[i],
            FMT(this->annotation_prefix, " round[%zu]", i));
    }

    // For last round, output to the result variable and add `key` to the
    // result, along with any add_to_result.
    round_results[NumRounds - 1] = result;

    if (add_to_result_is_valid) {
        libsnark::pb_linear_combination<FieldT> key_plus_add_to_result;
        key_plus_add_to_result.assign(this->pb, key + add_to_result);
        round_gadgets.emplace_back(
            this->pb,
            round_results[NumRounds - 2],
            key,
            round_constants[NumRounds - 1],
            round_results[NumRounds - 1],
            key_plus_add_to_result,
            FMT(this->annotation_prefix, " round[%zu]", NumRounds - 1));
    } else {
        round_gadgets.emplace_back(
            this->pb,
            round_results[NumRounds - 2],
            key,
            round_constants[NumRounds - 1],
            round_results[NumRounds - 1],
            key,
            FMT(this->annotation_prefix, " round[%zu]", NumRounds - 1));
    }
}

template<typename FieldT, size_t Exponent, size_t NumRounds>
MiMC_permutation_gadget<FieldT, Exponent, NumRounds>::MiMC_permutation_gadget(
    libsnark::protoboard<FieldT> &pb,
    const libsnark::pb_linear_combination<FieldT> &msg,
    const libsnark::pb_linear_combination<FieldT> &key,
    const libsnark::pb_variable<FieldT> &result,
    const std::string &annotation_prefix)
    : MiMC_permutation_gadget(
          pb,
          msg,
          key,
          result,
          libsnark::pb_linear_combination<FieldT>(),
          false,
          annotation_prefix)
{
}

template<typename FieldT, size_t Exponent, size_t NumRounds>
MiMC_permutation_gadget<FieldT, Exponent, NumRounds>::MiMC_permutation_gadget(
    libsnark::protoboard<FieldT> &pb,
    const libsnark::pb_linear_combination<FieldT> &msg,
    const libsnark::pb_linear_combination<FieldT> &key,
    const libsnark::pb_variable<FieldT> &result,
    const libsnark::pb_linear_combination<FieldT> &add_to_result,
    const std::string &annotation_prefix)
    : MiMC_permutation_gadget(
          pb, msg, key, result, add_to_result, true, annotation_prefix)
{
}

template<typename FieldT, size_t Exponent, size_t NumRounds>
void MiMC_permutation_gadget<FieldT, Exponent, NumRounds>::
    generate_r1cs_constraints()
{
    // For each round, generates the constraints for the corresponding round
    // gadget.
    for (auto &gadget : round_gadgets) {
        gadget.generate_r1cs_constraints();
    }
}

template<typename FieldT, size_t Exponent, size_t NumRounds>
void MiMC_permutation_gadget<FieldT, Exponent, NumRounds>::
    generate_r1cs_witness() const
{
    // For each round, generates the witness for the corresponding round
    // gadget.
    for (auto &gadget : round_gadgets) {
        gadget.generate_r1cs_witness();
    }
}

// The following constants correspond to the iterative computation of sha3_256
// hash function over the initial seed "clearmatics_mt_seed". See:
// client/zethCodeConstantsGeneration.py for more details
template<typename FieldT, size_t Exponent, size_t NumRounds>
void MiMC_permutation_gadget<FieldT, Exponent, NumRounds>::
    setup_sha3_constants()
{
    if (round_constants_initialized) {
        return;
    }

    // For simplicity, always generate constants for MaxRounds.
    round_constants.reserve(MaxRounds);

    // The constant is set to "0" in the first round of MiMC permutation (see:
    // https://eprint.iacr.org/2016/492.pdf)
    round_constants.push_back(FieldT("0"));

    // clang-format off

    // This is sha3_256(sha3_256("clearmatics_mt_seed"))
    round_constants.push_back(FieldT(
        "22159019873790129476324495190496603411493310235845550845393361088354059025587"));

    round_constants.push_back(FieldT(
        "27761654615899466766976328798614662221520122127418767386594587425934055859027"));
    round_constants.push_back(FieldT(
        "94824950344308939111646914673652476426466554475739520071212351703914847519222"));
    round_constants.push_back(FieldT(
        "84875755167904490740680810908425347913240786521935721949482414218097022905238"));
    round_constants.push_back(FieldT(
        "103827469404022738626089808362855974444473512881791722903435218437949312500276"));
    round_constants.push_back(FieldT(
        "79151333313630310680682684119244096199179603958178503155035988149812024220238"));
    round_constants.push_back(FieldT(
        "69032546029442066350494866745598303896748709048209836077355812616627437932521"));
    round_constants.push_back(FieldT(
        "71828934229806034323678289655618358926823037947843672773514515549250200395747"));
    round_constants.push_back(FieldT(
        "20380360065304068228640594346624360147706079921816528167847416754157399404427"));
    round_constants.push_back(FieldT(
        "33389882590456326015242966586990383840423378222877476683761799984554709177407"));
    round_constants.push_back(FieldT(
        "50122810070778420844700285367936543284029126632619100118638682958218725318756"));
    round_constants.push_back(FieldT(
        "49246859699528342369154520789249265070136349803358469088610922925489948122588"));
    round_constants.push_back(FieldT(
        "42301293999667742503298132605205313473294493780037112351216393454277775233701"));
    round_constants.push_back(FieldT(
        "84114918321547685007627041787929288135785026882582963701427252073231899729239"));
    round_constants.push_back(FieldT(
        "62442564517333183431281494169332072638102772915973556148439397377116238052032"));
    round_constants.push_back(FieldT(
        "90371696767943970492795296318744142024828099537644566050263944542077360454000"));
    round_constants.push_back(FieldT(
        "115430938798103259020685569971731347341632428718094375123887258419895353452385"));
    round_constants.push_back(FieldT(
        "113486567655643015051612432235944767094037016028918659325405959747202187788641"));
    round_constants.push_back(FieldT(
        "42521224046978113548086179860571260859679910353297292895277062016640527060158"));
    round_constants.push_back(FieldT(
        "59337418021535832349738836949730504849571827921681387254433920345654363097721"));
    round_constants.push_back(FieldT(
        "11312792726948192147047500338922194498305047686482578113645836215734847502787"));
    round_constants.push_back(FieldT(
        "5531104903388534443968883334496754098135862809700301013033503341381689618972"));
    round_constants.push_back(FieldT(
        "67267967506593457603372921446668397713655666818276613345969561709158934132467"));
    round_constants.push_back(FieldT(
        "14150601882795046585170507190892504128795190437985555320824531798948976631295"));
    round_constants.push_back(FieldT(
        "85062650450907709431728516509140931676564801299509460081586249478375415684322"));
    round_constants.push_back(FieldT(
        "3190636703526705373452173482292964566521687248139217048214149162895182633187"));
    round_constants.push_back(FieldT(
        "94697707246459731032848302079578714910941380385884087153796554334872238022178"));
    round_constants.push_back(FieldT(
        "105237079024348272465679804525604310926083869213267017956044692586513087552889"));
    round_constants.push_back(FieldT(
        "107666297462370279081061498341391155289817553443536637437225808625028106164694"));
    round_constants.push_back(FieldT(
        "50658185643016152702409617752847261961811370146977869351531768522548888496960"));
    round_constants.push_back(FieldT(
        "40194505239242861003888376856216043830225436269588275639840138989648733836164"));
    round_constants.push_back(FieldT(
        "18446023938001439123322925291203176968088321100216399802351969471087090508798"));
    round_constants.push_back(FieldT(
        "56716868411561319312404565555682857409226456576794830238428782927207680423406"));
    round_constants.push_back(FieldT(
        "99446603622401702299467002115709680008186357666919726252089514718382895122907"));
    round_constants.push_back(FieldT(
        "14440268383603206763216449941954085575335212955165966039078057319953582173633"));
    round_constants.push_back(FieldT(
        "19800531992512132732080265836821627955799468140051158794892004229352040429024"));
    round_constants.push_back(FieldT(
        "105297016338495372394147178784104774655759157445835217996114870903812070518445"));
    round_constants.push_back(FieldT(
        "25603899274511343521079846952994517772529013612481201245155078199291999403355"));
    round_constants.push_back(FieldT(
        "42343992762533961606462320250264898254257373842674711124109812370529823212221"));
    round_constants.push_back(FieldT(
        "10746157796797737664081586165620034657529089112211072426663365617141344936203"));
    round_constants.push_back(FieldT(
        "83415911130754382252267592583976834889211427666721691843694426391396310581540"));
    round_constants.push_back(FieldT(
        "90866605176883156213219983011392724070678633758652939051248987072469444200627"));
    round_constants.push_back(FieldT(
        "37024565646714391930474489137778856553925761915366252060067939966442059957164"));
    round_constants.push_back(FieldT(
        "7989471243134634308962365261048299254340659799910534445820512869869542788064"));
    round_constants.push_back(FieldT(
        "15648939481289140348738679797715724220399212972574021006219862339465296839884"));
    round_constants.push_back(FieldT(
        "100133438935846292803417679717817950677446943844926655798697284495340753961844"));
    round_constants.push_back(FieldT(
        "84618212755822467879717121296483255659772850854170590780922087915497421596465"));
    round_constants.push_back(FieldT(
        "66815981435852782130184794409662156021404245655267602728283138458689925010111"));
    round_constants.push_back(FieldT(
        "100011403138602452635630699813302791324969902443516593676764382923531277739340"));
    round_constants.push_back(FieldT(
        "57430361797750645341842394309545159343198597441951985629580530284393758413106"));
    round_constants.push_back(FieldT(
        "70240009849732555205629614425470918637568887938810907663457802670777054165279"));
    round_constants.push_back(FieldT(
        "115341201140672997375646566164431266507025151688875346248495663683620086806942"));
    round_constants.push_back(FieldT(
        "11188962021222070760150833399355814187143871338754315850627637681691407594017"));
    round_constants.push_back(FieldT(
        "22685520879254273934490401340849316430229408194604166253482138215686716109430"));
    round_constants.push_back(FieldT(
        "51189210546148312327463530170430162293845070064001770900624850430825589457055"));
    round_constants.push_back(FieldT(
        "14807565813027010873011142172745696288480075052292277459306275231121767039664"));
    round_constants.push_back(FieldT(
        "95539138374056424883213912295679274059417180869462186511207318536449091576661"));
    round_constants.push_back(FieldT(
        "113489397464329757187555603731541774715600099685729291423921796997078292946609"));
    round_constants.push_back(FieldT(
        "104312240868162447193722372229442001535106018532365202206691174960555358414880"));
    round_constants.push_back(FieldT(
        "8267151326618998101166373872748168146937148303027773815001564349496401227343"));
    round_constants.push_back(FieldT(
        "76298755107890528830128895628139521831584444593650120338808262678169950673284"));
    round_constants.push_back(FieldT(
        "73002305935054160156217464153178860593131914821282451210510325210791458847694"));
    round_constants.push_back(FieldT(
        "74544443080560119509560262720937836494902079641131221139823065933367514898276"));
    round_constants.push_back(FieldT(
        "36856043990250139109110674451326757800006928098085552406998173198427373834846"));
    // clang-format on

    assert(round_constants.size() == MaxRounds);
    round_constants_initialized = true;
}

} // namespace libzeth

#endif // __ZETH_CIRCUITS_MIMC_PERMUTATION_TCC__

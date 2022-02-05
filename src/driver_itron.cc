/*
 Copyright (C) 2022 Fredrik Öhrström

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include"meters_common_implementation.h"

using namespace std;

struct MeterItron : public virtual MeterCommonImplementation
{
    MeterItron(MeterInfo &mi, DriverInfo &di);

private:

    double total_water_consumption_m3_ {};
    string meter_date_;
    double target_water_consumption_m3_ {};
    string target_date_;
};

static bool ok = registerDriver([](DriverInfo&di)
{
    di.setName("itron");
    di.setMeterType(MeterType::WaterMeter);
    di.setExpectedTPLSecurityMode(TPLSecurityMode::AES_CBC_IV);
    di.addLinkMode(LinkMode::T1);
    di.addDetection(MANUFACTURER_ITW,  0x07,  0x03);
    di.setConstructor([](MeterInfo& mi, DriverInfo& di){ return shared_ptr<Meter>(new MeterItron(mi, di)); });
});

MeterItron::MeterItron(MeterInfo &mi, DriverInfo &di) : MeterCommonImplementation(mi, di)
{
    setMeterType(MeterType::WaterMeter);

    setExpectedTPLSecurityMode(TPLSecurityMode::AES_CBC_IV);

    addLinkMode(LinkMode::T1);

    addFieldWithExtractor(
        "total",
        Quantity::Volume,
        NoDifVifKey,
        VifScaling::Auto,
        MeasurementType::Instantaneous,
        ValueInformation::Volume,
        StorageNr(0),
        TariffNr(0),
        IndexNr(1),
        PrintProperty::JSON | PrintProperty::FIELD | PrintProperty::IMPORTANT,
        "The total water consumption recorded by this meter.",
        SET_FUNC(total_water_consumption_m3_, Unit::M3),
        GET_FUNC(total_water_consumption_m3_, Unit::M3));

    addStringFieldWithExtractor(
        "meter_date",
        Quantity::Text,
        NoDifVifKey,
        MeasurementType::Instantaneous,
        ValueInformation::DateTime,
        StorageNr(0),
        TariffNr(0),
        IndexNr(1),
        PrintProperty::JSON,
        "Date when measurement was recorded.",
        SET_STRING_FUNC(meter_date_),
        GET_STRING_FUNC(meter_date_));

    addFieldWithExtractor(
        "target",
        Quantity::Volume,
        NoDifVifKey,
        VifScaling::Auto,
        MeasurementType::Instantaneous,
        ValueInformation::Volume,
        StorageNr(1),
        TariffNr(0),
        IndexNr(1),
        PrintProperty::JSON | PrintProperty::FIELD | PrintProperty::IMPORTANT,
        "The total water consumption recorded at the beginning of this month.",
        SET_FUNC(target_water_consumption_m3_, Unit::M3),
        GET_FUNC(target_water_consumption_m3_, Unit::M3));

    addStringFieldWithExtractor(
        "target_date",
        Quantity::Text,
        NoDifVifKey,
        MeasurementType::Instantaneous,
        ValueInformation::Date,
        StorageNr(1),
        TariffNr(0),
        IndexNr(1),
        PrintProperty::JSON,
        "Date when target water consumption was recorded.",
        SET_STRING_FUNC(target_date_),
        GET_STRING_FUNC(target_date_));

}

// Test: SomeWater itron 12345698 NOKEY
// Comment: Test ITRON T1 telegram not encrypted, which has no 2f2f markers.
// telegram=|384497269856341203077AD90000A0#0413FD110000066D2C1AA1D521004413300F0000426CBF2C047F0000060C027F862A0E79678372082100|
// {"media":"water","meter":"itron","name":"SomeWater","id":"12345698","total_m3":4.605,"meter_date":"2022-01-21 01:26","target_m3":3.888,"target_date":"2021-12-31","timestamp":"2022-02-05T01:03:40Z"}
// |SomeWater;12345698;4.605000;3.888000;2022-02-05 03:05.05
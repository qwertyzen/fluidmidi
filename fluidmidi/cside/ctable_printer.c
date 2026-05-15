#include <stdio.h>
#include <string.h>
#include "cmidi_types.h"
#include "cmidiconst.h"
#include "cbytearray.h"
#include "utils.h"
#include "cmidi.h"

const char* MType_str(cMtype mtype)
{
    const char* mtypestr[] = {"EMPTY", "META", "TEMPO", "TSIG", "TRACK", "SYSEX", "ESC",
                               "PC", "CC", "POLYP", "CHANP", "CHAN",
                               "BEND1", "NOF", "BEND0", "NON",
                               "REST", "EOT", "REC"};
    return mtypestr[mtype];
}

static const char* get_meta_name(uint8_t status)
{
    char *meta_name = "META_NAME";
    switch (status) {
        case _SEQUENCE_NUMBER:
            meta_name = "SEQUENCE_NUMBER";
            break;
        case _TEXT           :
            meta_name = "TEXT";
            break;
        case _COPYRIGHT      :
            meta_name = "COPYRIGHT";
            break;
        case _TRACK_NAME     :
            meta_name = "TRACK_NAME";
            break;
        case _INSTRUMENT_NAME:
            meta_name = "INSTRUMENT_NAME";
            break;
        case _LYRICS         :
            meta_name = "LYRICS";
            break;
        case _MARKER         :
            meta_name = "MARKER";
            break;
        case _CUE_MARKER     :
            meta_name = "CUE_MARKER";
            break;
        case _DEVICE_NAME    :
            meta_name = "DEVICE_NAME";
            break;
        case _CHANNEL_PREFIX :
            meta_name = "CHANNEL_PREFIX";
            break;
        case _MIDI_PORT      :
            meta_name = "MIDI_PORT";
            break;
        case _END_OF_TRACK   :
            meta_name = "END_OF_TRACK";
            break;
        case _TEMPO_SET      :
            meta_name = "TEMPO_SET";
            break;
        case _SMPTE_OFFSET   :
            meta_name = "SMPTE_OFFSET";
            break;
        case _TIME_SIGNATURE :
            meta_name = "TIME_SIGNATURE";
            break;
        case _KEY_SIGNATURE  :
            meta_name = "KEY_SIGNATURE";
            break;
        case _SEQUENCER_SPECIFIC:
            meta_name = "SEQUENCER_SPECIFIC";
    }
    return meta_name;
}

// Row printer API
static void row_print_color(MidiRow *row)
{
    switch(row->mtype) {
        case MTYPE_META:
        case MTYPE_TEMPO:
        case MTYPE_TSIG:
        case MTYPE_TRACK:
        case MTYPE_SYSEX:
        case MTYPE_ESC:
        case MTYPE_EOT:
        case MTYPE_REST:
            printf(ANSIRED);
            break;
        case MTYPE_PC:
        case MTYPE_BEND0:
        case MTYPE_BEND1:
        case MTYPE_POLYP:
        case MTYPE_CHANP:
        case MTYPE_EMPTY:
        case MTYPE_CHAN:
        case MTYPE_REC:
            printf(ANSIMAGENTA);
            break;
        case MTYPE_CC:
            printf(ANSIBLUE);
            break;
        case MTYPE_NOF:
            printf(ANSIYELLOW);
            break;
        case MTYPE_NON:
            printf(ANSIGREEN);
            break;
    }
}

static void row_print_header(MidiRow *row)
{
    printf("%d %s:\t", row->time, MType_str(row->mtype));
}

static int PRINTER_OPT_BYTEARRAY = 0;

static void row_print_footer(MidiRow *row)
{
    printf("\t\t0x%02X [%u, %u]" ANSIEND, row->status, row->track, row->channel);
    if (PRINTER_OPT_BYTEARRAY) {
        printf("(@%p) |", row->data); bytes_print(row->data); printf("| "); printf("%u", row->delta);
    }
}

static void row_print_meta(MidiRow *row)
{
    printf("%s\t\t(", get_meta_name(row->status));
    bytes_print(row->data);
    printf(")\t");
}

static void row_print_meta_text(MidiRow *row)
{
    printf("%s\t\t'", get_meta_name(row->status));
    bytes_print_txt(row->data);
    printf("'\t");
}

static void row_print_trackname(MidiRow *row)
{
    row_print_meta_text(row);
}

static void row_print_temposet(MidiRow *row)
{
    printf("%s\t\t", get_meta_name(row->status));
    printf("tempo = %.2f bpm", ctempo2bpm(row->pitch));
}

static void row_print_timesig(MidiRow *row)
{
    printf("%s\t\t", get_meta_name(row->status));
    printf("%d | %d", row->program, row->control);
}

static void row_print_program(MidiRow *row)
{
    printf("%d\t%s", row->program, program_changes[row->program]);
}

static void row_print_control(MidiRow *row)
{
    printf("%d\t%s\t%d", row->control, control_changes[row->control], row->cc_val);
}

static void row_print_non(MidiRow *row)
{
    printf("%d\t%d\t%d\t", row->midi_d, row->noteval, row->notevel);
}

static void row_print_nof(MidiRow *row)
{
    printf("\t%d\t%d\t", row->noteval, row->notevel);
}

static void row_print_bend(MidiRow *row)
{
    printf("%d\t", row->pitch);
}

void print_row(MidiRow *row)
{
    row_print_color(row);
    row_print_header(row);
    switch (row->mtype) {
        case MTYPE_CC:
            row_print_control(row);
            break;
        case MTYPE_BEND1:
        case MTYPE_BEND0:
            row_print_bend(row);
            break;
        case MTYPE_NOF:
            row_print_nof(row);
            break;
        case MTYPE_NON:
            row_print_non(row);
            break;
        case MTYPE_META:
            switch (row->status) {
                case _TEXT:
                case _COPYRIGHT:
                case _TRACK_NAME:
                case _INSTRUMENT_NAME:
                case _LYRICS:
                case _MARKER:
                case _CUE_MARKER:
                case _DEVICE_NAME:
                    row_print_meta_text(row);
                    break;
                default:
                    row_print_meta(row);
            }
            break;
        case MTYPE_TEMPO:
            row_print_temposet(row);
            break;
        case MTYPE_TSIG:
            row_print_timesig(row);
            break;
        case MTYPE_EOT:
            row_print_meta(row);
            break;
        case MTYPE_TRACK:
            row_print_trackname(row);
            break;
        case MTYPE_PC:
            row_print_program(row);
            break;
        default:
            printf("Not implemented: data ");
            bytes_print(row->data);
            printf(", delta=%d",row->delta);
    }
    row_print_footer(row);
    printf("\n");
    fflush(stdout);
}

void ctable_print(MidiRow *table, int num_rows, int print_bytes)
{
    PRINTER_OPT_BYTEARRAY = print_bytes;
    int i;
    for (i=0; i<num_rows; i++) {
        print_row(&table[i]);
    }
}

void ctable_compare_print(MidiRow *table1, MidiRow *table2, long num_rows)
{
    long i;
    for (i = 0; i < num_rows; i++) {
        printf("a: ");
        print_row(&table1[i]);
        printf("b: ");
        print_row(&table2[i]);
        printf("\n");
    }
}
